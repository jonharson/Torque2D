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
#include "SQLiteEvents.h"

/// How much time we assume is enough to finish working with the SQLiteRecordSet
/// It will trigger a warning in the console for every SQLiteRecordSet found inside SQLiteInterface (which are expired)
#define SQLITE_RECORDSET_EXPIRE_TIMEOUT_MS (1024*60)

class Thread;
class Semaphore;

class SQLiteInterface : public SimGroup
{
protected:
   typedef SimGroup Parent;

public:
   SQLiteInterface();
   ~SQLiteInterface();

   virtual bool onAdd();
   virtual void onRemove();
   DECLARE_CONOBJECT(SQLiteInterface);
   static void consoleInit();

   S32 processExclusiveSQLiteQuery( ExclusiveSQLiteSelectQueryRequest *request );

public:
   /// Place an event in internal SQLiteInterface's event queue
   virtual void postSQLEvent( SQLiteRequest &event );

   /// Static thread startup method
   static void processThread( void *udata );

protected:
   /// Main process loop
   void processLoop();

   /// Process a single event from the current queue
   S32 processSQLiteEvent( SQLiteRequest *event );

   /// Process a generic query
   S32 processSQLiteQuery( GenericSQLiteQueryRequest *request );
   /// Process a generic select query
   S32 processGenericSQLiteSelectQuery( GenericSQLiteSelectQueryRequest *request );
   /// Process a generic update query
   S32 processGenericSQLiteUpdateQuery( GenericSQLiteUpdateQueryRequest *request );
   /// Process example query
   S32 processSQLiteExampleQuery( ExampleSQLiteQueryRequest *request ); // SQLiExample

private:
   /// Pointer to the actual sqlite3 object
   sqlite3 *mSQLiteDB;
   /// Filename of the SQLite3 database
   const char *mDatabase;

   /// Events are stored here by any thread, for processing by the main thread
   Vector<SQLiteRequest*> m_SQLiteEventQueue1, m_SQLiteEventQueue2, *mp_CurrentEventQueue;

   /// The queue mutex
   void *m_SQLiteMutex;
   /// Exclusive mutex
   void *m_SQLiteExclusiveMutex;
   /// This semaphore is triggered when queue has been updated (added new task)
   Semaphore *m_ActionSemaphore;
   /// The actual worker thread
   Thread* m_SQLiteProcessThread;

   /// Indicates are we still working or need to shutdown
   volatile U32 mv_ShuttingDown;
   /// Do we have successfully initialized?
   volatile U32 mv_Initialized;
   /// Set to true to output more information into console
   volatile U32 mv_DebugMode;

   /// Open SQLite3 database (called in thread). An error is automatically handled
   S32 _openSQLiteDB();
   /// Close the database before shutting down the thread
   void _closeSQLiteDB();
   /// Retrieves error from sqlite3 engine and dumps it to the console
   void _dumpError( S32 errorCode = -1 );

   inline bool isShuttingDown()
   {
      return dAtomicRead(mv_ShuttingDown) != 0;
   }
   inline void setInitialized( bool mode )
   {
      dCompareAndSwap(mv_Initialized, !mode, mode);
   }

public:
   void initialize( const char *database );

   inline bool isInitialized()
   {
      return dAtomicRead(mv_Initialized) != 0;
   };
   inline bool isDebug()
   {
      return dAtomicRead(mv_DebugMode) != 0;
   }
   inline void setDebug( bool debugMode )
   {
      dCompareAndSwap(mv_DebugMode, !debugMode, debugMode);
   };
};

#endif // _SQLITE_INTERFACE_H_
