//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
// Copyright Dedicated Logic 2011
//-----------------------------------------------------------------------------

#ifndef _SQLITE_RECORDSET_
#define _SQLITE_RECORDSET_

#ifndef _SIMBASE_H_
#include "sim/simBase.h"
#endif

#include "memory/safeDelete.h"

class SQLiteInterface;

class SQLiteRecordSet : public SimObject
{
private:
   typedef SimObject Parent;

public:
   SQLiteRecordSet( SQLiteInterface *sqli = NULL );
   ~SQLiteRecordSet();

public:
   virtual bool onAdd();
   virtual void onRemove();

   static void initPersistFields();

public:
   S32 getDataFieldCount() { return mDataFieldCount; }
   void setDataFieldName( S32 index, const char* fieldName );
   const char* getDataFieldName( U32 index );
   const char* getDataFieldValue( U32 index );
   const char* getDataFieldValue( const char* pFieldName );
   static S32 _sqliteRowAddCallback(void *obj, S32 argc, char **argv, char **columnNames);

public:
   bool nextRecord();
   void rewindToFirstRecord() { mFirstRecord = true; };

   struct SQLiteDataRow
   {
      VectorPtr<char*> mValues;
      ~SQLiteDataRow()
      {
         // Clear values
         for(VectorPtr<char*>::iterator i = mValues.begin(); i != mValues.end(); i++)
		 {
            SAFE_DELETE_ARRAY(*i);
		 }
         mValues.clear();
      }
   };

   struct SQLiteDataSet
   {
      SQLiteDataSet()
      {
         mValid = false;
         mCurrentRow = -1;
         mCurrentColumn = -1;
         mNumRows = 0;
         mNumCols = 0;
      }
      ~SQLiteDataSet()
      {
         // Clear field names
         for(VectorPtr<char*>::iterator i = mFieldNames.begin(); i != mFieldNames.end(); i++)
		 {
            SAFE_DELETE_ARRAY(*i);
		 }
         mFieldNames.clear();
         // Clear data rows
         for(VectorPtr<SQLiteDataRow*>::iterator i = mRows.begin(); i != mRows.end(); i++)
		 {
            SAFE_DELETE(*i);
		 }
         mRows.clear();
      }
      bool mValid;
      S32 mCurrentRow;
      S32 mCurrentColumn;
      S32 mNumRows;
      S32 mNumCols;
      VectorPtr<char*> mFieldNames;
      VectorPtr<SQLiteDataRow*> mRows;
   };
   typedef VectorPtr<SQLiteDataRow*> SQLiteDataRowType;
   SQLiteDataRowType::iterator mCurrentRecord;

private:
   SQLiteDataSet* mDataSet;
   SimObjectPtr<SQLiteInterface> mSQLiteInterface;
   bool isDebug();

   S32 mDataFieldCount;
   bool mFirstRecord;
   bool mBusy;
   bool mIgnoreExpiredWarning;
   U32 mInitiatedAt;
   U32 mExpiryWarnCounter;
   U32 mLastExpiryCheck;
   S32 mAffectedRows;
   U64 mLastID;

public:
   DECLARE_CONOBJECT(SQLiteRecordSet);
   void checkExpiryStatus();

   // How many records do we have
   S32 mRecordCount;
   S32 getRecordCount() { return mRecordCount; }

   // The last inserted ID (for auto-increment)
   void setLastInsertID(U64 id) { mLastID = id; };
   U64 getLastInsertID() { return mLastID; };

   // Number of rows affected by the query
   void setAffectedRows(S32 num) { mAffectedRows = num; };
   S32 getAffectedRows() { return mAffectedRows; };

   // Query result
   bool mSucceed;

   void setBusy( bool busy ) { mBusy = busy; }
   bool isBusy() { return mBusy; }

   static S32 sgNextSQLiteQueryId;

   const char *mReqSQLString;
   const char *mReqQueryCallback;
   SimObjectId mReqQueryRef; 
   void processReplyEvent();
};

#endif // _SQLITE_RECORDSET_
