//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
// Copyright Dedicated Logic 2011
//-----------------------------------------------------------------------------

#include "sqliteInterface.h"
#include "SQLiteRecordSet.h"

//#include "console/engineAPI.h"

S32 SQLiteInterface::processSQLiteEvent( SQLiteRequest *event )
{
   if(!mSQLiteDB)
      return -1;
   if(isDebug())
      Con::printf("%d SQLiteInterface::processEvent %d", Sim::getCurrentTime(), event->type);
   try
   {
      switch(event->type)
      {
      case GenericQuery:
         return processSQLiteQuery((GenericSQLiteQueryRequest*)event);
         break;
      case GenericSelectQuery:
         return processGenericSQLiteSelectQuery((GenericSQLiteSelectQueryRequest*)event);
         break;
      case GenericUpdateQuery:
         return processGenericSQLiteUpdateQuery((GenericSQLiteUpdateQueryRequest*)event);
         break;
      case ExampleQuery:
         // SQLiExample
         //return processSQLiteExampleQuery((ExampleSQLiteQueryRequest*)event);
         break;
      }
   }
   catch(...)
   {
      _dumpError();
      Con::errorf("SQLiteInterface::processEvent: Database thread threw an un-handled exception.");
   }
   return SQLITE_OK;
}

S32 SQLiteRecordSet::_sqliteRowAddCallback( void *obj, S32 argc, char **argv, char **columnNames )
{
   // Nothing found?
   if(argc == 0)
      return SQLITE_OK;

   // Not sure if this can ever happen, but we need to be sure everything is "ok", as we run in a thread and don't want any weird crashes/memory corruption.
   if(!obj)
      return -1;

   // basically this callback is called for each row in the SQL query result.
   // for each row, argc indicates how many columns are returned.
   // columnNames[i] is the name of the column
   // argv[i] is the value of the column

   SQLiteRecordSet * recordSet = (SQLiteRecordSet*)obj;
   // If this is a first result we receive, we need to be sure we have initialized dataSet so we can keep the data
   if(!recordSet->mDataSet)
   {
      recordSet->mDataSet = new SQLiteDataSet();
      // Initialize the amount of fields on recordSet
      recordSet->mDataFieldCount = recordSet->mDataSet->mNumCols = argc;
      // Fill the field names
      for (S32 i = 0; i < argc; i++)
      {
         char * name = new char[dStrlen(columnNames[i]) + 1];
         dStrcpy(name, columnNames[i]);
         recordSet->mDataSet->mFieldNames.push_back(name);
      }
   }

   // create a new result row
   SQLiteDataRow * row = new SQLiteDataRow;
   // loop through all the columns and stuff them into our row
   for (S32 i = 0; i < argc; i++)
   {
      if(recordSet->isDebug())
         Con::printf("SQLiteRecordSet::_sqliteRowAddCallback(%d): %s = %s", recordSet->getId(), columnNames[i], argv[i] ? argv[i] : "NULL");
      if(argv[i])
      {
         char * val = new char[dStrlen(argv[i]) + 1];
         dStrcpy(val, argv[i]);
         row->mValues.push_back(val);
      }
      else
      {
         row->mValues.push_back(dStrdup(""));
      }
   }
   recordSet->mRecordCount++;
   recordSet->mDataSet->mRows.push_back(row);

   return SQLITE_OK;
}

void SQLiteRecordSet::processReplyEvent()
{
   if(mReqQueryCallback)
   {
      // We need to call method on object?
      if(mReqQueryRef)
      {
         SimObject* refObj = NULL;
         // We need to find an object and be sure that it was not deleted while the sqlite3 was processing the SQL request
         if(!Sim::findObject(mReqQueryRef, refObj))
         {
            Con::errorf("SQLiteRecordSet::processReplyEvent -- Can't find reference object (deleted?), results [%d] cleared!", getId());
            Con::warnf("SQL request was: %s", mReqSQLString);
            deleteObject();
         }
         else
         {
            if(mReqQueryRef == getId())
               Con::warnf("Calling the method on SQLiteRecordSet is a bad idea! If you perform %result.delete(); inside this scope, you will crash! Better to use SQLiteInterface instead.");
            if(refObj->isMethod(mReqQueryCallback))
               Con::executef(refObj, 2, mReqQueryCallback, getIdString());
            else
            {
               Con::errorf("SQLiteRecordSet::processReplyEvent -- Can't find console method %s for %d {%s}! Results [%d] cleared!", 
                  mReqQueryCallback, refObj->getId(), refObj->getClassName(), getId());
               Con::warnf("SQL request was: %s", mReqSQLString);
               deleteObject();
            }
         }
      }
      else
      {
         // Global function!
         if(Con::isFunction(mReqQueryCallback))
            Con::executef(2, mReqQueryCallback, getIdString());
         else
         {
            Con::errorf("SQLiteRecordSet::processReplyEvent -- Can't find console function %s! Results [%d] cleared!", mReqQueryCallback, getId());
            Con::warnf("SQL request was: %s", mReqSQLString);
            deleteObject();
         }
      }
   }
   else
   {
      // If none, use the default
      if(Con::isFunction("SQLiClearResult"))
         Con::executef(2, "SQLiClearResult", getIdString());
      else
         deleteObject();
   }
}

class sqliGeneralReplyEvent : public SimEvent
{
public:
   sqliGeneralReplyEvent() {}
   ~sqliGeneralReplyEvent() {}
   virtual void process(SimObject *object)
   {
      SQLiteRecordSet* caller = (SQLiteRecordSet*)(object);
      caller->setBusy(false);
      caller->processReplyEvent();
   }
};

class SQLiteRecordSetErrorEvent : public SimEvent
{
   S32 mErrorCode;
   const char *mError;
public:
   SQLiteRecordSetErrorEvent(S32 errorCode = -1, const char *errorMessage = NULL)
   {
      mErrorCode = errorCode;
      if(errorMessage)
      {
         mError = (char *)dMalloc(dStrlen(errorMessage)+1);
         dMemset((void*)mError, 0, dStrlen(errorMessage)+1);
         dMemcpy((void*)mError, (void*)errorMessage, dStrlen(errorMessage));
      }
      else
         mError = NULL;

   }
   ~SQLiteRecordSetErrorEvent()
   {
      SAFE_FREE(mError);
   }
   virtual void process( SimObject *object )
   {
      SQLiteRecordSet *sqlrs = dynamic_cast<SQLiteRecordSet*>(object);
      if(sqlrs)
      {
         if(sqlrs->isMethod("onError"))
            Con::executef(sqlrs, 3, "onError", avar("%d", mErrorCode), mError);
         else
         {
            Con::errorf("SQLiteInterface [%d] (%s) error!", sqlrs->getId(), sqlrs->getName());
            Con::warnf("SQL request was: %s", sqlrs->mReqSQLString);
         }
      }
      else
         Con::errorf("Error: SQLiteRecordSet object not found!");
   }
};

S32 SQLiteInterface::processExclusiveSQLiteQuery( ExclusiveSQLiteSelectQueryRequest *request )
{
   if(!mSQLiteDB)
      return -1;
   S32 result = 0;
   Mutex::lockMutex(m_SQLiteExclusiveMutex);
   char *errorString = NULL;
   if(isDebug())
      Con::printf("SQLiteInterface::processSQLiteQuery");
   result = sqlite3_exec(mSQLiteDB, request->mRecordSet->mReqSQLString, SQLiteRecordSet::_sqliteRowAddCallback, (void*)request->mRecordSet, &errorString);
   if(result == SQLITE_OK)
   {
      // Just in case we ran an "INSERT" query, we retrieve the last ID sqlite generated for us
      request->mRecordSet->setLastInsertID(sqlite3_last_insert_rowid(mSQLiteDB));
      // Just in case we ran an "INSERT", "UPDATE" or "DELETE" query, we retrieve the affected rows
      request->mRecordSet->setAffectedRows(sqlite3_changes(mSQLiteDB));
      // Post the event to the main thread
      request->mRecordSet->mSucceed = true;
   }
   else
   {
      Con::errorf("SQLiteInterface [%d] (%s) error!", getId(), getName());
      Con::warnf("SQL request was: %s", errorString);
      request->mRecordSet->mSucceed = false;
      sqlite3_free(errorString);
   }
   Mutex::unlockMutex(m_SQLiteExclusiveMutex);
   return result;
}

S32 SQLiteInterface::processSQLiteQuery( GenericSQLiteQueryRequest *request )
{
   if(!mSQLiteDB)
      return -1;
   S32 result = -1;
   char *errorString = NULL;
   if(isDebug())
      Con::printf("SQLiteInterface::processSQLiteQuery");
   result = sqlite3_exec(mSQLiteDB, request->mRecordSet->mReqSQLString, SQLiteRecordSet::_sqliteRowAddCallback, (void*)request->mRecordSet, &errorString);
   if(result == SQLITE_OK)
   {
      // Just in case we ran an "INSERT" query, we retrieve the last ID sqlite generated for us
      request->mRecordSet->setLastInsertID(sqlite3_last_insert_rowid(mSQLiteDB));
      // Just in case we ran an "INSERT", "UPDATE" or "DELETE" query, we retrieve the affected rows
      request->mRecordSet->setAffectedRows(sqlite3_changes(mSQLiteDB));
      // Post the event to the main thread
      Sim::postEvent(request->mRecordSet, new sqliGeneralReplyEvent(), Sim::getTargetTime());
   }
   else
   {
      Sim::postEvent(this, new SQLiteRecordSetErrorEvent(result, errorString), Sim::getTargetTime());
      sqlite3_free(errorString);
   }
   return result;
}

S32 SQLiteInterface::processGenericSQLiteSelectQuery( GenericSQLiteSelectQueryRequest *request )
{
   if(!mSQLiteDB)
      return -1;
   S32 result = -1;
   char *errorString = NULL;
   if(isDebug())
      Con::printf("SQLiteInterface::processGenericSQLiteSelectQuery");
   result = sqlite3_exec(mSQLiteDB, request->mRecordSet->mReqSQLString, SQLiteRecordSet::_sqliteRowAddCallback, (void*)request->mRecordSet, &errorString);
   if(result == SQLITE_OK)
   {
      // Post the event to the main thread
      Sim::postEvent(request->mRecordSet, new sqliGeneralReplyEvent(), Sim::getTargetTime());
   }
   else
   {
      Sim::postEvent(this, new SQLiteRecordSetErrorEvent(result, errorString), Sim::getTargetTime());
      sqlite3_free(errorString);
   }
   return result;
}

S32 SQLiteInterface::processGenericSQLiteUpdateQuery( GenericSQLiteUpdateQueryRequest *request )
{
   if(!mSQLiteDB)
      return -1;
   S32 result = -1;
   char *errorString = NULL;
   if(isDebug())
      Con::printf("SQLiteInterface::processGenericSQLiteUpdateQuery");
   result = sqlite3_exec(mSQLiteDB, request->mRecordSet->mReqSQLString, SQLiteRecordSet::_sqliteRowAddCallback, (void*)request->mRecordSet, &errorString);
   if(result == SQLITE_OK)
   {
      // We performed the modification to the data, store the changes:
      // last ID sqlite generated for us (if it was INSERT call)
      request->mRecordSet->setLastInsertID(sqlite3_last_insert_rowid(mSQLiteDB));
      // affected rows
      request->mRecordSet->setAffectedRows(sqlite3_changes(mSQLiteDB));
      // Post the event to the main thread
      Sim::postEvent(request->mRecordSet, new sqliGeneralReplyEvent(), Sim::getTargetTime());
   }
   else
   {
      Sim::postEvent(this, new SQLiteRecordSetErrorEvent(result, errorString), Sim::getTargetTime());
      sqlite3_free(errorString);
   }
   return result;
}
