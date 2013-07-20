//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
// Copyright Dedicated Logic 2011
//-----------------------------------------------------------------------------

#include "SQLiteRecordSet.h"

IMPLEMENT_CONOBJECT(SQLite::RecordSet);

S32 SQLite::RecordSet::sgNextSQLiteQueryId = 1;

SQLite::RecordSet::RecordSet(SQLite::Interface *sqli) :
	mDataFieldCount(0),
	mRecordCount(0),
	mFirstRecord(true),
	mBusy(false)
{
   mInterface = sqli;
   mLastID = 0;
   mAffectedRows = 0;
   mSucceed = true;
   mExpiryWarnCounter = 0;
   mIgnoreExpiredWarning = false;
   mInitiatedAt = 0;
   mLastExpiryCheck = 0;

   mReqSQLString = NULL;
   mReqQueryCallback = NULL;
   mReqQueryRef = NULL;

   mCurrentRecord = 0;
   mDataSet = NULL;
}

SQLite::RecordSet::~RecordSet(void)
{
   SAFE_DELETE(mDataSet);
   SAFE_DELETE(mReqSQLString);
   SAFE_DELETE(mReqQueryCallback);
}

bool SQLite::RecordSet::onAdd(void)
{
   if (!Parent::onAdd())
      return false;

   // Set it to current time, so we can track down how much time it took to exec the SQL,
   // this is also used to track the "missed" results, like if someone forget to delete the result after finished working with it
   mInitiatedAt = Platform::getRealMilliseconds();
   mLastExpiryCheck = mInitiatedAt;

   return true;
}

void SQLite::RecordSet::onRemove(void)
{
   Parent::onRemove();
}

void SQLite::RecordSet::initPersistFields(void)
{
   Parent::initPersistFields();
}

void SQLite::RecordSet::checkExpiryStatus(void)
{
   if(mIgnoreExpiredWarning)
      return;

   mLastExpiryCheck = Platform::getRealMilliseconds();
   const U32 elapsedMS = mLastExpiryCheck - mLastExpiryCheck;

   if(elapsedMS > SQLITE_RECORDSET_EXPIRE_TIMEOUT_MS)
   {
      mExpiryWarnCounter++;

      Con::warnf("Warning: found an outdated SQLite::RecordSet [%d] inside SQLI [%d] (%s). Time elapsed: %d sec / Warn counter: %d ",
         getId(), mInterface->getId(), mInterface->getName(),
         mExpiryWarnCounter * SQLITE_RECORDSET_EXPIRE_TIMEOUT_MS,
         mExpiryWarnCounter);

      if(isDebug() && getFieldDictionary())
         getFieldDictionary()->printFields(this);
   }
}

bool SQLite::RecordSet::isDebug(void)
{
   if(mInterface.isNull())
      return false;

   return mInterface->isDebug();
}

bool SQLite::RecordSet::nextRecord(void)
{
   // Don't try to do anything in case we don't have results (was performing INSERT/UPDATE/DELETE maybe?)
   if(mDataFieldCount == 0)
      return false;

   // If this is a first call...
   if(mFirstRecord)
   {
      mFirstRecord = false;
      // Jump to first row
      mCurrentRecord = mDataSet->mRows.begin();
   }
   else
   {
      // Skip to the next, but only if we are not at the end already
      if(mCurrentRecord != mDataSet->mRows.end())
         ++mCurrentRecord;
   }

   // If we still have records, update values
   bool haveRecord = mCurrentRecord != mDataSet->mRows.end();
   if(haveRecord)
   {
      for(S32 i=0; i< mDataFieldCount; i++)
         setDataField(StringTable->insert(mDataSet->mFieldNames[i]), NULL, (*mCurrentRecord)->mValues[i]);
   }

   return haveRecord;
}

const char* SQLite::RecordSet::getDataFieldName(U32 index)
{
   if(index < 0 || index > mDataSet->mFieldNames.size()-1)
      return "";
   else
      return mDataSet->mFieldNames[index];
}

const char* SQLite::RecordSet::getDataFieldValue(U32 index)
{
   if (mFirstRecord)
   {
      Con::errorf("Call SQLite::RecordSet::nextRecord() before calling getDataFieldValue");
      return "";
   }

   if(index < 0 || index > (*mCurrentRecord)->mValues.size()-1)
   {
      Con::errorf("SQLite::RecordSet::getFieldValue error: out of range. Requested field with index %d, but we have %d fields in dataset.", index, mDataSet->mFieldNames.size());
      return "";
   }

   return (*mCurrentRecord)->mValues[index];
}

const char* SQLite::RecordSet::getDataFieldValue(const char* fieldName)
{
   for(U32 i=0; i < mDataSet->mFieldNames.size(); i++)
   {
      if(dStricmp(fieldName, mDataSet->mFieldNames[i]) == 0)
         return getDataFieldValue(i);
   }

   Con::errorf("SQLite::RecordSet::getDataFieldValue error: field `%s` not found!", fieldName);
   return "";
}

/*

//TODO!

DefineEngineMethod(SQLiteRecordSet, getSQL, const char*, (),,
                   "@return original SQL request string\n"
                   "@ingroup SQLiteInterface")
{
   return Con::getReturnBuffer(object->mReqSQLString);
}

DefineEngineMethod(SQLiteRecordSet, getLastInsertId, const char *, (),,
                   "@return The last value used for an AUTO_INCREMENT field of the INSERT command\n"
                   "@ingroup SQLiteInterface")
{
   if(object->isBusy())
   {
      Con::errorf("SQLiteRecordSet::getLastInsertId() failed: The SQLiteRecordSet is not yet finished processing!");
      return false;
   }
   char * buf = Con::getArgBuffer(32);
   dSprintf(buf, 32, "%u", object->getLastInsertID());
   return buf;
}

DefineEngineMethod(SQLiteRecordSet, getAffectedRowsCount, S32, (),,
                   "@return The number of rows affected by the query (INSERT/UPDATE/DELETE request)\n"
                   "@ingroup SQLiteInterface")
{
   if(object->isBusy())
   {
      Con::errorf("SQLiteRecordSet::getAffectedRowsCount() failed: The SQLiteRecordSet is not yet finished processing!");
      return false;
   }
   return object->getAffectedRows();
}

DefineEngineMethod(SQLiteRecordSet, getRecordCount, S32, (),,
                   "@return The total number of records received from the SQLite3 by the SQL request.\n"
                   "@ingroup SQLiteInterface")
{
   if(object->isBusy())
   {
      Con::errorf("SQLiteRecordSet::getRecordCount() failed: The SQLiteRecordSet is not yet finished processing!");
      return -1;
   }
   return object->getRecordCount();
}

DefineEngineMethod(SQLiteRecordSet, nextRecord, bool, (),,
                   "Move to the next record on the dataset.\n"
                   "@return true if we have a valid data, false when finished scrolling.\n"
                   "@tsexample\n"
                      "while(!%record.nextRecord())\n"
                      "   processMyRequest(%record);\n"
                      "%record.delete();"
                   "@endtsexample\n"
                   "@ingroup SQLiteInterface")
{
   if(object->isBusy())
   {
      Con::errorf("SQLiteRecordSet::nextRecord() failed: The SQLiteRecordSet is not yet finished processing!");
      return false;
   }
   return object->nextRecord();
}

DefineEngineMethod(SQLiteRecordSet, rewind, void, (),,
                   "Rewind the recordSet to the first record\n"
                   "@ingroup SQLiteInterface")
{
   if(object->isBusy())
   {
      Con::errorf("SQLiteRecordSet::rewind() failed: The SQLiteRecordSet is not yet finished processing!");
      return;
   }
   object->rewindToFirstRecord();
}

DefineEngineMethod(SQLiteRecordSet, getDataFieldCount, S32, (),,
                   "@return Amount of fields retrieved from the database.\n"
                   "@ingroup SQLiteInterface")
{
   if(object->isBusy())
   {
      Con::errorf("SQLiteRecordSet::getDataFieldCount() failed: The SQLiteRecordSet is not yet finished processing!");
      return -1;
   }
   return object->getDataFieldCount();
}

DefineEngineMethod(SQLiteRecordSet, getDataFieldName, const char*, (S32 index),,
                   "@return Field name for specified index.\n"
                   "@ingroup SQLiteInterface")
{
   if(object->isBusy())
   {
      Con::errorf("SQLiteRecordSet::getDataFieldName() failed: The SQLiteRecordSet is not yet finished processing!");
      return "";
   }
   return Con::getReturnBuffer(object->getDataFieldName(index));
}

DefineEngineMethod(SQLiteRecordSet, getDataFieldValue, const char *, (const char * IndexOrFieldName),,
                   "@return Field value from specified index or field name\n"
                   "@ingroup SQLiteInterface")
{
   if(object->isBusy())
   {
      Con::errorf("SQLiteRecordSet::getDataFieldValue() failed: The SQLiteRecordSet is not yet finished processing!");
      return "";
   }
   if (dIsalpha(IndexOrFieldName[0]))
      return Con::getReturnBuffer(object->getDataFieldValue(IndexOrFieldName));
   else
      return Con::getReturnBuffer(object->getDataFieldValue(IndexOrFieldName));
}
*/