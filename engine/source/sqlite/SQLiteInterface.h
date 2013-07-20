//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
// Copyright Dedicated Logic 2011
//-----------------------------------------------------------------------------
// Credits:
// Tony Richards for the `Event Driven Database` resource
// John Vanderbeck for the `SQLite Integration for Torque` resource
// Those two resource has been taken into account while we have developed this
// And of course - thanks to GarageGames team for such a great engine, as Torque is
//-----------------------------------------------------------------------------

#ifndef _SQLITE_INTERFACE_H_
#define _SQLITE_INTERFACE_H_

#ifndef _SIMBASE_H_
	#include "sim/simBase.h"
#endif

#ifndef _PLATFORMINTRINSICS_H_
	#include "platform/platformIntrinsics.h"
#endif

#include "sqlite3.h"

/// How much time we assume is enough to finish working with the SQLiteRecordSet
/// It will trigger a warning in the console for every SQLiteRecordSet found inside SQLiteInterface (which are expired)
#define SQLITE_RECORDSET_EXPIRE_TIMEOUT_MS (1024*60)

class Thread;
class Semaphore;

namespace SQLite
{
	struct Request;
	struct GenericQueryRequest;
	struct GenericSelectQueryRequest;
	struct GenericUpdateQueryRequest;
	/*
	struct DalObjectSelectQueryRequest;
	struct DalObjectUpdateQueryRequest;
	struct DalTableSelectQueryRequest;
	struct DalTableUpdateQueryRequest;
	*/
	struct ExclusiveQueryRequest;

	class Interface : public SimGroup
	{
	public:
	   Interface(void);
	   ~Interface(void);

	   virtual bool onAdd(void);
	   virtual void onRemove(void);

	   DECLARE_CONOBJECT(SQLite::Interface);

	   static void consoleInit(void);

	   S32 processExclusiveQuery(SQLite::ExclusiveQueryRequest* request);

	   /// Place an event in internal SQLiteInterface's event queue
	   virtual void postSQLEvent(SQLite::Request& event);

	   /// Static thread startup method
	   static void processThread(void* udata);

	   void initialize(const char* database);

	   inline bool isInitialized()
	   {
		  return dAtomicRead(mInitialized) != 0;
	   };

	   inline bool isDebug(void)
	   {
		  return dAtomicRead(mDebugMode) != 0;
	   }

	   inline void setDebug(bool debugMode)
	   {
		  dCompareAndSwap(mDebugMode, !debugMode, debugMode);
	   };

	protected:
	   typedef SimGroup Parent;

	   /// Main process loop
	   void processLoop(void);

	   /// Process a single event from the current queue
	   S32 processEvent(SQLite::Request* event);
	   S32 processQuery(SQLite::GenericQueryRequest* request);
	   S32 processGenericSelectQuery(SQLite::GenericSelectQueryRequest* request);
	   S32 processGenericUpdateQuery(SQLite::GenericUpdateQueryRequest* request);
	   /*
	   S32 processDalObjectSelectQuery(SQLite::DalObjectSelectQueryRequest* request);
	   S32 processDalObjectUpdateQuery(SQLite::DalObjectUpdateQueryRequest* request);
	   S32 processDalTableSelectQuery(SQLite::DalTableSelectQueryRequest* request);
	   S32 processDalTableUpdateQuery(SQLite::DalTableUpdateQueryRequest* request);
	   */

	private:
	   /// Open SQLite3 database (called in thread). An error is automatically handled
	   S32 _openSQLiteDB(void);
	   /// Close the database before shutting down the thread
	   void _closeSQLiteDB(void);
	   /// Retrieves error from sqlite3 engine and dumps it to the console
	   void _dumpError(S32 errorCode = -1);

	   inline bool isShuttingDown(void)
	   {
		  return dAtomicRead(mShuttingDown) != 0;
	   }

	   inline void setInitialized(bool mode)
	   {
		  dCompareAndSwap(mInitialized, !mode, mode);
	   }

	   /// Pointer to the actual sqlite3 object
	   sqlite3* mSQLiteDB;
	   const char* mDatabaseName;

	   /// Events are stored here by any thread, for processing by the main thread
	   Vector<SQLite::Request*> mEventQueue1;
	   Vector<SQLite::Request*> mEventQueue2;
	   Vector<SQLite::Request*>* mCurrentEventQueue;

	   /// The queue mutex
	   void* mQueueMutex;
	   /// Exclusive mutex
	   void* mExclusiveMutex;
	   /// This semaphore is triggered when queue has been updated (added new task)
	   Semaphore* mActionSemaphore;
	   /// The actual worker thread
	   Thread* mProcessThread;

	   /// Indicates are we still working or need to shutdown
	   volatile U32 mShuttingDown;
	   /// Do we have successfully initialized?
	   volatile U32 mInitialized;
	   /// Set to true to output more information into console
	   volatile U32 mDebugMode;
	};
}

#endif // _SQLITE_INTERFACE_H_
