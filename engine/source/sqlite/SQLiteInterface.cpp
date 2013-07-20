//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
// Copyright Dedicated Logic 2011
//-----------------------------------------------------------------------------

#include "sqliteInterface.h"

#include "console/consoleInternal.h"
#include "sim/simEvent.h"
#include "console/console.h"
#include "platform/threads/semaphore.h"
#include "platform/threads/thread.h"

#include "sqlite3.h"
#include "SQLiteRecordSet.h"
#include "SQLiteEvents.h"

IMPLEMENT_CONOBJECT(SQLite::Interface);

S32 gSQLICount = 0;

SQLite::Interface::Interface(void) :
	mCurrentEventQueue(&mEventQueue1),
	mQueueMutex(Mutex::createMutex()),
	mExclusiveMutex(Mutex::createMutex()),
	mActionSemaphore(new Semaphore(0)),
	mShuttingDown(false),
	mInitialized(false),
	mDebugMode(false),
	mSQLiteDB(NULL),
	mDatabaseName(NULL)
{
   gSQLICount++;
}

SQLite::Interface::~Interface(void)
{
   // We need to perform some clean-up!
   Mutex::destroyMutex(mExclusiveMutex);
   Mutex::destroyMutex(mQueueMutex);
   SAFE_DELETE(mActionSemaphore);
   SAFE_FREE(mDatabaseName);

   gSQLICount--;
}

namespace SQLite
{
	class sqliCheckForOldResultsEvent : public SimEvent
	{
	public:
	   sqliCheckForOldResultsEvent(void) { };
	   ~sqliCheckForOldResultsEvent(void) { };

	   void process(SimObject* object)
	   {
		  if(object == NULL) 
			  return;

		  SQLite::Interface *sqli = dynamic_cast<SQLite::Interface*>(object);

		  if(!sqli) 
			  return;

		  for(SimObjectList::iterator i = sqli->begin(); i != sqli->end(); i++)
		  {
			 SQLite::RecordSet* rs = (SQLite::RecordSet*)(*i);
			 // Check if the object expired
			 rs->checkExpiryStatus();
		  }
		  Sim::postEvent(sqli, new sqliCheckForOldResultsEvent(), Sim::getTargetTime() + SQLITE_RECORDSET_EXPIRE_TIMEOUT_MS);
	   }
	};
}

bool SQLite::Interface::onAdd(void)
{
   if(!Parent::onAdd())
      return false;

   // Issue the initial event. It will re-submit itself again and again.
   Sim::postEvent(this, new sqliCheckForOldResultsEvent(), Sim::getTargetTime() + SQLITE_RECORDSET_EXPIRE_TIMEOUT_MS);

   return true;
}

void SQLite::Interface::onRemove(void)
{
   if(isInitialized())
   {
      if(isDebug())
         Con::warnf("Shutting down the SQLite::Interface [%d] (%s)", getId(), getName());

      // We need to shutdown the worker thread
      dCompareAndSwap(mShuttingDown, false, true);
      // We need to move thread out of the semaphore block, so it can get out and shutdown properly
      mActionSemaphore->release();
      // Cleanup!
      _closeSQLiteDB();
      setInitialized(false);
   }

   // Check if we have any SQLiteRecordSet objects
   if(size())
   {
      Con::errorf("SQLite::Interface has been removed from the Sim, but it still contains %d objects!", size());
      Con::warnf("All of the SQLite::RecordSet objects found has been moved to the `SQLiteResultsGroup` (SimGroup).");

      SimGroup *sqliResultsGroup = NULL;
      if(!Sim::findObject("SQLiteResultsGroup", sqliResultsGroup))
      {
         sqliResultsGroup = new SimGroup();
         sqliResultsGroup->registerObject("SQLiteResultsGroup");

         while(size())
            sqliResultsGroup->addObject((*this)[0]);
      }
   }

   Parent::onRemove();
}

void SQLite::Interface::consoleInit(void)
{
	/*

	//TODO!

   Con::addConstant("$SQLITE::UNKNOWNERROR", -1, "Unknown/unrecognized error\n" "@ingroup SQLiteInterface");
   // Error codes retrieved from sqlite3 engine
   Con::addConstant("$SQLITE::OK", SQLITE_OK, "Successful result\n" "@ingroup SQLiteInterface");
   Con::addConstant("$SQLITE::ERROR", SQLITE_ERROR, "SQL error or missing database\n" "@ingroup SQLiteInterface");
   Con::addConstant("$SQLITE::INTERNAL", SQLITE_INTERNAL, "Internal logic error in SQLite\n" "@ingroup SQLiteInterface");
   Con::addConstant("$SQLITE::PERM", SQLITE_PERM, "Access permission denied\n" "@ingroup SQLiteInterface");
   Con::addConstant("$SQLITE::ABORT", SQLITE_ABORT, "Callback routine requested an abort\n" "@ingroup SQLiteInterface");
   Con::addConstant("$SQLITE::BUSY", SQLITE_BUSY, "The database file is locked\n" "@ingroup SQLiteInterface");
   Con::addConstant("$SQLITE::LOCKEd", SQLITE_LOCKED, "A table in the database is locked\n" "@ingroup SQLiteInterface");
   Con::addConstant("$SQLITE::NOMEM", SQLITE_NOMEM, "A malloc() failed\n" "@ingroup SQLiteInterface");
   Con::addConstant("$SQLITE::READONLY", SQLITE_READONLY, "Attempt to write a readonly database\n" "@ingroup SQLiteInterface");
   Con::addConstant("$SQLITE::INTERRUPT", SQLITE_INTERRUPT, "Operation terminated by sqlite3_interrupt()\n" "@ingruop SQLiteInterface");
   Con::addConstant("$SQLITE::IOERR", SQLITE_IOERR, "Some kind of disk I/O error occurred\n" "@ingroup SQLiteInterface");
   Con::addConstant("$SQLITE::CORRUPT", SQLITE_CORRUPT, "The database disk image is malformed\n" "@ingroup SQLiteInterface");
   Con::addConstant("$SQLITE::NOTFOUND", SQLITE_NOTFOUND, "Unknown opcode in sqlite3_file_control()\n" "@ingroup SQLiteInterface");
   Con::addConstant("$SQLITE::FULL", SQLITE_FULL, "Insertion failed because database is full\n" "@ingroup SQLiteInterface");
   Con::addConstant("$SQLITE::CANTOPEN", SQLITE_CANTOPEN, "Unable to open the database file\n" "@ingroup SQLiteInterface");
   Con::addConstant("$SQLITE::PROTOCOL", SQLITE_PROTOCOL, "Database lock protocol error\n" "@ingroup SQLiteInterface");
   Con::addConstant("$SQLITE::EMPTY", SQLITE_EMPTY, "Database is empty\n" "@ingroup SQLiteInterface");
   Con::addConstant("$SQLITE::SCHEMA", SQLITE_SCHEMA, "The database schema changed\n" "@ingroup SQLiteInterface");
   Con::addConstant("$SQLITE::TOOBIG", SQLITE_TOOBIG, "String or BLOB exceeds size limit\n" "@ingroup SQLiteInterface");
   Con::addConstant("$SQLITE::CONSTRAINT", SQLITE_CONSTRAINT, "Abort due to constraint violation\n" "@ingroup SQLiteInterface");
   Con::addConstant("$SQLITE::MISMATCH", SQLITE_MISMATCH, "Data type mismatch\n" "@ingroup SQLiteInterface");
   Con::addConstant("$SQLITE::MISUSE", SQLITE_MISUSE, "Library used incorrectly\n" "@ingroup SQLiteInterface");
   Con::addConstant("$SQLITE::NOLFS", SQLITE_NOLFS, "Uses OS features not supported on host\n" "@ingroup SQLiteInterface");
   Con::addConstant("$SQLITE::AUTH", SQLITE_AUTH, "Authorization denied\n" "@ingroup SQLiteInterface");
   Con::addConstant("$SQLITE::FORMAT", SQLITE_FORMAT, "Auxiliary database format error\n" "@ingroup SQLiteInterface");
   Con::addConstant("$SQLITE::RANGE", SQLITE_RANGE, "2nd parameter to sqlite3_bind out of range\n" "@ingroup SQLiteInterface");
   Con::addConstant("$SQLITE::NOTADB", SQLITE_NOTADB, "File opened that is not a database file\n" "@ingroup SQLiteInterface");
   Con::addConstant("$SQLITE::ROW", SQLITE_ROW, "sqlite3_step() has another row ready\n" "@ingroup SQLiteInterface");
   Con::addConstant("$SQLITE::DONE", SQLITE_DONE, "sqlite3_step() has finished executing\n" "@ingroup SQLiteInterface");
   */

   Parent::consoleInit();
}

void SQLite::Interface::_closeSQLiteDB(void)
{
   // Perform some clean-up
   Mutex::lockMutex(mExclusiveMutex);
   Mutex::lockMutex(mQueueMutex);

   for(U32 i=0; i < mEventQueue1.size(); i++)
   {
      SAFE_FREE(mEventQueue1[i]);
   }
   mEventQueue1.clear();

   for(U32 i=0; i < mEventQueue2.size(); i++)
   {
      SAFE_FREE(mEventQueue2[i]);
   }
   mEventQueue2.clear();

   Mutex::unlockMutex(mQueueMutex);

   if(mSQLiteDB)
      sqlite3_close(mSQLiteDB);

   Mutex::unlockMutex(mExclusiveMutex);
   mSQLiteDB = NULL;
}

namespace SQLite
{
	class sqliCantOpenEvent : public SimEvent
	{
	public:
	   sqliCantOpenEvent(S32 errorCode = -1, const char *errorMessage = NULL)
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

	   ~sqliCantOpenEvent(void)
	   {
		  SAFE_FREE(mError);
	   }

	   virtual void process(SimObject* object)
	   {
		  SQLite::Interface *sqli = dynamic_cast<SQLite::Interface*>(object);
		  if(sqli)
		  {
			 if(sqli->isMethod("onOpenFailed"))
				Con::executef(sqli, 3, "onOpenFailed", avar("%d", mErrorCode), mError);
			 else
				Con::errorf("SQLite::Interface [%d] (%s) error: can't open database!", sqli->getId(), sqli->getName());
		  }
		  else
			 Con::errorf("Error: SQLite::Interface object not found!");
	   }

	private:
	   	S32 mErrorCode;
		const char *mError;
	};

	class sqliOpenEvent : public SimEvent
	{
	public:
	   sqliOpenEvent(void) {}
	   ~sqliOpenEvent(void) {}

	   virtual void process(SimObject* object)
	   {
		  SQLite::Interface *sqli = dynamic_cast<SQLite::Interface*>(object);
		  if(sqli)
		  {
			 if(sqli->isMethod("onInitialized"))
				Con::executef(sqli, 1, "onInitialized");
			 else
				Con::printf("SQLite::Interface [%d] (%s) initialized", sqli->getId(), sqli->getName());
		  }
		  else
			 Con::errorf("Error: SQLite::Interface object not found!");
	   }
	};

	class sqliGeneralErrorEvent : public SimEvent
	{
	public:
	   sqliGeneralErrorEvent(S32 errorCode = -1, const char *errorMessage = NULL)
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

	   ~sqliGeneralErrorEvent(void)
	   {
		  SAFE_FREE(mError);
	   }

	   virtual void process(SimObject* object)
	   {
		  SQLite::Interface *sqli = dynamic_cast<SQLite::Interface*>(object);
		  if(sqli)
		  {
			 if(sqli->isMethod("onError"))
				Con::executef(sqli, 3, "onError", avar("%d", mErrorCode), mError);
			 else
				Con::errorf("SQLite::Interface [%d] (%s) error!", sqli->getId(), sqli->getName());
		  }
		  else
			 Con::errorf("Error: SQLite::Interface object not found!");
	   }

	private:
	   S32 mErrorCode;
	   const char *mError;
	};
}

void SQLite::Interface::_dumpError(S32 errorCode)
{
   char* errorMsg = (char *)sqlite3_errmsg(mSQLiteDB);
   Sim::postEvent(this, new sqliGeneralErrorEvent(errorCode, errorMsg), Sim::getTargetTime());
   sqlite3_free(errorMsg);
}

S32 SQLite::Interface::_openSQLiteDB()
{
   S32 result = sqlite3_open(mDatabaseName, &mSQLiteDB);

   if(result != SQLITE_OK)
   {
      char* errorMsg = (char *)sqlite3_errmsg(mSQLiteDB);
      Sim::postEvent(this, new sqliCantOpenEvent(result, errorMsg), Sim::getTargetTime());
      sqlite3_free(errorMsg);
   }
   else
   {
      setInitialized(true);
      Sim::postEvent(this, new sqliOpenEvent(), Sim::getTargetTime());
   }

   return result;
}

void SQLite::Interface::postSQLEvent(Request& event)
{
   if(!isInitialized())
   {
      Con::errorf("Can't post SQLite::Request as the SQLite::Interface [%d] is not initialized!", this->getId());
      return;
   }

   if(isShuttingDown())
   {
      Con::errorf("Can't post SQLite::Request as the SQLite::Interface [%d] is shutting down!", this->getId());
      return;
   }

   if(isDebug())
      Con::errorf("SQLite::Request::postSQLEvent: %d", S32(event.type));

   Mutex::lockMutex(mQueueMutex);
   // Create a deep copy of event, and save a pointer to the copy in a vector.
   SQLite::Request* copy = (SQLite::Request*)dMalloc(event.size);
   dMemcpy(copy, &event, event.size);
   mCurrentEventQueue->push_back(copy);
   Mutex::unlockMutex(mQueueMutex);
   // Trigger the semaphore, so the thread can process the event
   mActionSemaphore->release();
}

void SQLite::Interface::processLoop()
{
   while(!mShuttingDown)
   {
      // Wait for the event to be signaled, so we don't run for nothing
      mActionSemaphore->acquire();

      Mutex::lockMutex(mQueueMutex);

      // swap event queue pointers
      Vector<SQLite::Request*>& fullEventQueue = *mCurrentEventQueue;

      if(mCurrentEventQueue == &mEventQueue1)
         mCurrentEventQueue = &mEventQueue2;
      else
         mCurrentEventQueue = &mEventQueue1;

      if(isDebug())
         Con::printf("SQLite::Interface::processLoop -- processing %d elements from the %s queue.", (mCurrentEventQueue == &mEventQueue1) ? "first" : "second");

      Mutex::unlockMutex(mQueueMutex);

      // Keep track of the original size
      const int size = fullEventQueue.size();
      // Walk the event queue in fifo order, processing the events, then clear the queue.
      for(S32 i=0; i < size; i++)
      {
         Mutex::lockMutex(mExclusiveMutex);
         S32 result = processEvent(fullEventQueue[i]);

         if(result != SQLITE_OK)
            _dumpError(result);

         SAFE_FREE(fullEventQueue[i]);
         Mutex::unlockMutex(mExclusiveMutex);
      }

      fullEventQueue.clear();
   }
}

void SQLite::Interface::processThread(void* udata)
{
   SQLite::Interface* pThis = (SQLite::Interface*)udata;
   const S32 result = pThis->_openSQLiteDB();

   if(pThis->isDebug())
      Con::printf("SQLite::Interface [%d] (%s) initialization status: %d", pThis->getId(), pThis->getName(), result);

   if(result != SQLITE_OK)
   {
      pThis->_dumpError(result);
      return;
   }

   pThis->processLoop();
}

void SQLite::Interface::initialize(const char *database)
{
   if(isInitialized())
   {
      Con::errorf("SQLite::Interface::initialize() error: SQLite::Interface is already initialized!");
      return;
   }

   if(isShuttingDown())
   {
      Con::errorf("SQLite::Interface::initialize() error: SQLite::Interface [%d] is shutting down!", this->getId());
      return;
   }

   // Sanity check
   if(!Platform::isFile(database))
   {
      Con::errorf("SQLite::Interface::initialize() failed: can't find %s", database);
      return;
   }

   // Save the path to the database file
   mDatabaseName = (char*)dMalloc(dStrlen(database)+1);
   dMemset((void*)mDatabaseName, 0, dStrlen(database)+1);
   dMemcpy((void*)mDatabaseName, (void*)database, dStrlen(database));

   // Create a new thread
   mProcessThread = new Thread((ThreadRunFunction)processThread, this);
   // Auto-delete thread when finished.
   mProcessThread->autoDelete = true;
   // Ready, steady, go!
   mProcessThread->start();
}

/*

//TODO!

ConsoleFunction(getSQLiteInterfaceCount, S32, (),,
                      "Returns a total amount of SQLiteInterface objects created in engine.\n"
                      "@return total amount of SQLiteInterface objects created in engine"
                      "@ingroup SQLiteInterface")
{
   return gSQLICount;
}

ConsoleMethod(SQLiteInterface, initialize, void, (const char *database),,
                    "Initialize SQLiteInterface (start the thread and open specified database).\n"
                    "@param database a filename of the SQLite3 database.\n"
                    "@ingroup SQLiteInterface")
{
   object->initialize(database);
}

ConsoleMethod(SQLiteInterface, isInitialized, bool, (),,
                    "Returns true if the SQLiteInterface successfully initialized.\n"
                    "@return bool value of the initialization status.\n"
                    "@ingroup SQLiteInterface")
{
   return object->isInitialized();
}

ConsoleMethod(SQLiteInterface, setDebug, void, (bool debug),,
                    "Pass true to enable the debug mode of the SQLiteInterface.\n"
                    "@param debug pass true to output debug information, false to disable.\n"
                    "@ingroup SQLiteInterface")
{
   return object->setDebug(debug);
}

ConsoleMethod(SQLiteInterface, Execute, S32, (const char *sql, const char *callback, SimObject *queryRef), (NULL, NULL),
                    "Submit a query to SQLiteInterface for processing.\n"
                    "@param sql The SQL request the SQLite3 should process.\n"
                    "@param callback The console function or method (if queryRef is a valid SimObject) to call after the query is processed.\n"
                    "@param queryRef SimObject to call the callback on, can be NULL - in this case it will call global function.\n"
                    "@return SQLiteRecordSet object with results.\n"
                    "@ingroup SQLiteInterface")
{
   GenericSQLiteQueryRequest request;
   request.mRecordSet = new SQLiteRecordSet(object);
   request.mRecordSet->registerObject();
   request.mRecordSet->mReqSQLString = dStrdup(sql);
   request.mRecordSet->mReqQueryCallback = dStrdup(callback);
   if(queryRef)
      request.mRecordSet->mReqQueryRef = queryRef->getId();
   else
      request.mRecordSet->mReqQueryRef = 0;
   request.mRecordSet->setBusy(true);
   object->addObject(request.mRecordSet);
   object->postSQLEvent(request);
   return request.mRecordSet->getId();
}

ConsoleMethod(SQLiteInterface, SelectSQL, S32, (const char *sql, const char *callback, SimObject *queryRef), (NULL),
                    "Submit a query (select) to SQLiteInterface for processing.\n"
                    "@param sql The SQL request the SQLite3 should process.\n"
                    "@param callback The console function or method (if queryRef is a valid SimObject) to call after the query is processed.\n"
                    "@param queryRef SimObject to call the callback on, can be NULL - in this case it will call global function.\n"
                    "@return SQLiteRecordSet object with results.\n"
                    "@ingroup SQLiteInterface")
{
   GenericSQLiteSelectQueryRequest request;
   request.mRecordSet = new SQLiteRecordSet(object);
   request.mRecordSet->registerObject();
   request.mRecordSet->mReqSQLString = dStrdup(sql);
   request.mRecordSet->mReqQueryCallback = dStrdup(callback);
   if(queryRef)
      request.mRecordSet->mReqQueryRef = queryRef->getId();
   else
      request.mRecordSet->mReqQueryRef = 0;
   request.mRecordSet->setBusy(true);
   object->addObject(request.mRecordSet);
   object->postSQLEvent(request);
   return request.mRecordSet->getId();
}

ConsoleMethod(SQLiteInterface, UpdateSQL, S32, (const char *sql, const char *callback, SimObject *queryRef), (NULL, NULL),
                    "Submit a query (update/delete/insert) to SQLiteInterface for processing.\n"
                    "@param sql The SQL request the SQLite3 should process.\n"
                    "@param callback The console function or method (if queryRef is a valid SimObject) to call after the query is processed.\n"
                    "@param queryRef SimObject to call the callback on, can be NULL - in this case it will call global function."
                    "@return SQLiteRecordSet object with results.\n"
                    "@ingroup SQLiteInterface")
{
   GenericSQLiteUpdateQueryRequest request;
   request.mRecordSet = new SQLiteRecordSet(object);
   request.mRecordSet->registerObject();
   request.mRecordSet->mReqSQLString = dStrdup(sql);
   request.mRecordSet->mReqQueryCallback = dStrdup(callback);
   if(queryRef)
      request.mRecordSet->mReqQueryRef = queryRef->getId();
   else
      request.mRecordSet->mReqQueryRef = 0;
   request.mRecordSet->setBusy(true);
   object->addObject(request.mRecordSet);
   object->postSQLEvent(request);
   return request.mRecordSet->getId();
}
*/