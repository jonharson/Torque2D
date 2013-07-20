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

#include "SQLiteInterface.h"

namespace SQLite
{
	class RecordSet : public SimObject
	{
	public:
	   RecordSet(SQLite::Interface* sqli = NULL);
	   ~RecordSet(void);

	   DECLARE_CONOBJECT(SQLite::RecordSet);

	   virtual bool onAdd(void);
	   virtual void onRemove(void);

	   static void initPersistFields(void);

	   S32 getDataFieldCount(void) { return mDataFieldCount; }
	   void setDataFieldName(S32 index, const char* fieldName);
	   const char* getDataFieldName(U32 index);
	   const char* getDataFieldValue(U32 index);
	   const char* getDataFieldValue(const char* pFieldName);

	   static S32 _sqliteRowAddCallback(void *obj, S32 argc, char **argv, char **columnNames);

	   bool nextRecord(void);
	   void rewindToFirstRecord(void) { mFirstRecord = true; };

	   void checkExpiryStatus(void);

	   // How many records do we have
	   S32 mRecordCount;
	   S32 getRecordCount(void) { return mRecordCount; }

	   // The last inserted ID (for auto-increment)
	   void setLastInsertID(U64 id) { mLastID = id; };
	   U64 getLastInsertID(void) { return mLastID; };

	   // Number of rows affected by the query
	   void setAffectedRows(S32 num) { mAffectedRows = num; };
	   S32 getAffectedRows(void) { return mAffectedRows; };

	   // Query result
	   bool mSucceed;

	   void setBusy(bool busy) { mBusy = busy; }
	   bool isBusy(void) { return mBusy; }

	   static S32 sgNextSQLiteQueryId;

	   const char *mReqSQLString;
	   const char *mReqQueryCallback;
	   SimObjectId mReqQueryRef; 

	   void processReplyEvent(void);

	   struct DataRow
	   {
		  ~DataRow(void)
		  {
			 for(VectorPtr<char*>::iterator i = mValues.begin(); i != mValues.end(); i++)
			 {
				SAFE_DELETE_ARRAY(*i);
			 }
			 mValues.clear();
		  }

		  VectorPtr<char*> mValues;
	   };

	   struct DataSet
	   {
		  DataSet(void)
		  {
			 mValid = false;
			 mCurrentRow = -1;
			 mCurrentColumn = -1;
			 mNumRows = 0;
			 mNumCols = 0;
		  }

		  ~DataSet(void)
		  {
			 // Clear field names
			 for(VectorPtr<char*>::iterator i = mFieldNames.begin(); i != mFieldNames.end(); i++)
			 {
				SAFE_DELETE_ARRAY(*i);
			 }
			 mFieldNames.clear();
			 
			 // Clear data rows
			 for(VectorPtr<SQLite::RecordSet::DataRow*>::iterator i = mRows.begin(); i != mRows.end(); i++)
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
		  VectorPtr<SQLite::RecordSet::DataRow*> mRows;
	   };

	   typedef VectorPtr<SQLite::RecordSet::DataRow*> DataRowType;
	   DataRowType::iterator mCurrentRecord;

	private:
	   bool isDebug(void);

	   typedef SimObject Parent;

	   DataSet* mDataSet;
	   SimObjectPtr<SQLite::Interface> mInterface;

	   S32 mDataFieldCount;
	   bool mFirstRecord;
	   bool mBusy;
	   bool mIgnoreExpiredWarning;
	   U32 mInitiatedAt;
	   U32 mExpiryWarnCounter;
	   U32 mLastExpiryCheck;
	   S32 mAffectedRows;
	   U64 mLastID;
	};
}

#endif // _SQLITE_RECORDSET_
