//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
// Copyright Dedicated Logic 2011
//-----------------------------------------------------------------------------

#include "SQLiteInterface.h"
#include "SQLiteRecordSet.h"
#include "collection/hashTable.h"

//#include "core/util/str.h"
//#include "console/arrayObject.h"
//#include "console/engineAPI.h"

static S32 _sqliteExampleCallback(void *obj, S32 argc, char **argv, char **columnNames)
{
   // You can skip the object if you haven't specified it, but than be sure not to touch it, or at least add a check
   HashMap<const char*, char*>* ao = (HashMap<const char*, char*>*)obj;
   // We know the actual request and we sure the result is here with only two columns.
   // You may perform some additional check like:
   if(argc != 2)
      return -1;
   // Save the results
   ao->insert(argv[0], argv[1]);
   return SQLITE_OK;
}

// The method, where we do what we need to do with the data
S32 SQLiteInterface::processSQLiteExampleQuery( ExampleSQLiteQueryRequest *request )
{
   // Check if the sqlite3 is still valid object
   if(!mSQLiteDB)
      return -1;
   S32 result = -1;
   char *errorString = NULL;
   if(isDebug())
      Con::printf("SQLiteInterface::processSQLiteExampleQuery");
   result = sqlite3_exec(mSQLiteDB, "SELECT name, sql FROM sqlite_master WHERE type='table';", _sqliteExampleCallback, (void*)request->ao, &errorString);
   if(result == SQLITE_OK)
   {
      // If you want, you can perform any additional stuff here.
      // We are here, when the data has been processed and sqlite3 engine finished working on our request (all data has been given to use via callback).
      Con::printf("Wohoo!!! We are done!");
      Con::printf("Here is what we got:");
      Con::printf(" %32s | %s", "Table Name", "SQL");
	  for(HashMap<const char*, char*>::iterator i = request->ao->begin(); i != request->ao->end(); i++)
		  Con::printf(" %32s | %s", i->key, i->value);
      Con::warnf("Finished!");
      Con::warnf("You can see how the GenericSQLiteQueryRequest(s) are processed -- the data is sent to the MainThread via SimEvent for further processing");
      Con::warnf("Keep in mind we are still in a separate thread!!!");
      // We haven't registered ArrayObject, so we don't need to "unregister" it and it's safe to delete it now/here
      delete request->ao;
      request->ao = NULL;
   }
   else
   {
      Con::errorf("Oops!!! We got an error while retrieving data from the sqlite3 engine! Error code was %d, and the actual message is: `%s`", result, errorString);
      sqlite3_free(errorString);
   }
   return result;
}

/*
ConsoleMethod(SQLiteInterface, dumpSQLiteTables, void, (),,
                    "Dumps information about tables in the currently-used database.\n"
                    "@ingroup SQLiteInterface")
{
   ExampleSQLiteQueryRequest request;
   request.ao = new ArrayObject();
   object->postSQLEvent(request);
}

ConsoleMethod(SQLiteInterface, processSQL, S32, (const char * sql),,
                    "Run sql request directly from main thread.\n"
                    "@return SimId for the SQLiteRecordSet object with the data in it\n"
                    "@ingroup SQLiteInterface")
{
   ExclusiveSQLiteSelectQueryRequest request;
   request.mRecordSet = new SQLiteRecordSet(object);
   request.mRecordSet->registerObject();
   request.mRecordSet->mReqSQLString = dStrdup(sql);
   request.mRecordSet->mReqQueryRef = 0;
   request.mRecordSet->setBusy(true);
   object->addObject(request.mRecordSet);
   object->processExclusiveSQLiteQuery(&request);
   return request.mRecordSet->getId();
}
*/