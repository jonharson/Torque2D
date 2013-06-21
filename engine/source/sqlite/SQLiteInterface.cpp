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

IMPLEMENT_CONOBJECT(SQLiteInterface);

S32 gSQLICount = 0;

SQLiteInterface::SQLiteInterface()
:  mp_CurrentEventQueue(&m_SQLiteEventQueue1)
,  m_SQLiteMutex(Mutex::createMutex())
,  m_SQLiteExclusiveMutex(Mutex::createMutex())
,  m_ActionSemaphore(new Semaphore(0))
,  mv_ShuttingDown(false)
,  mv_Initialized(false)
,  mv_DebugMode(false)
,  mSQLiteDB(NULL)
,  mDatabase(NULL)
{
   gSQLICount++;
}

SQLiteInterface::~SQLiteInterface()
{
   // We need to perform some clean-up!
   Mutex::destroyMutex(m_SQLiteExclusiveMutex);
   Mutex::destroyMutex(m_SQLiteMutex);
   SAFE_DELETE(m_ActionSemaphore);
   SAFE_FREE(mDatabase);
   gSQLICount--;
}

class sqliCheckForOldResultsEvent : public SimEvent
{
public:
   sqliCheckForOldResultsEvent() { };
   ~sqliCheckForOldResultsEvent() { };
   void process(SimObject *obj)
   {
      if(obj == NULL) return;
      SQLiteInterface *sqli = (SQLiteInterface*)(obj);
      if(!sqli) return;
      for( SimObjectList::iterator i = sqli->begin(); i != sqli->end(); i++ )
      {
         SQLiteRecordSet *rs = (SQLiteRecordSet*)(*i);
         // Check if the object expired
         rs->checkExpiryStatus();
      }
      Sim::postEvent(sqli, new sqliCheckForOldResultsEvent(), Sim::getTargetTime() + SQLITE_RECORDSET_EXPIRE_TIMEOUT_MS);
   }
};

bool SQLiteInterface::onAdd()
{
   if (!Parent::onAdd())
      return false;

   // Issue the initial event. It will re-submit itself again and again.
   Sim::postEvent(this, new sqliCheckForOldResultsEvent(), Sim::getTargetTime() + SQLITE_RECORDSET_EXPIRE_TIMEOUT_MS);

   return true;
}

void SQLiteInterface::onRemove()
{
   if(isInitialized())
   {
      if(isDebug())
         Con::warnf("Shutting down the SQLiteInterface [%d] (%s)", getId(), getName());
      // We need to shutdown the worker thread
      dCompareAndSwap(mv_ShuttingDown, false, true);
      // We need to move thread out of the semaphore block, so it can get out and shutdown properly
      m_ActionSemaphore->release();
      // Cleanup!
      _closeSQLiteDB();
      setInitialized(false);
   }

   // Check if we have any SQLiteRecordSet objects
   if(size())
   {
      Con::errorf("SQLiteInterface has been removed from the Sim, but it still contains %d objects!", size());
      Con::warnf("All of the SQLiteRecordSet objects found has been moved to the `SQLiteResultsGroup` (SimGroup).");
      SimGroup *sqliResultsGroup = NULL;
      if(!Sim::findObject("SQLiteResultsGroup", sqliResultsGroup))
      {
         sqliResultsGroup = new SimGroup();
         sqliResultsGroup->registerObject("SQLiteResultsGroup");
         while(size())
            sqliResultsGroup->addObject( (*this)[0] );
      }
   }

   Parent::onRemove();
}

void SQLiteInterface::consoleInit()
{
	/*
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

void SQLiteInterface::_closeSQLiteDB()
{
   // Perform some clean-up
   Mutex::lockMutex(m_SQLiteExclusiveMutex);
   Mutex::lockMutex(m_SQLiteMutex);

   for(U32 i=0; i < m_SQLiteEventQueue1.size(); i++)
   {
      SAFE_FREE(m_SQLiteEventQueue1[i]);
   }
   m_SQLiteEventQueue1.clear();

   for(U32 i=0; i < m_SQLiteEventQueue2.size(); i++)
   {
      SAFE_FREE(m_SQLiteEventQueue2[i]);
   }
   m_SQLiteEventQueue2.clear();

   Mutex::unlockMutex(m_SQLiteMutex);
   if(mSQLiteDB)
      sqlite3_close(mSQLiteDB);
   Mutex::unlockMutex(m_SQLiteExclusiveMutex);
   mSQLiteDB = NULL;
}

class SQLiteCantOpenEvent : public SimEvent
{
   S32 mErrorCode;
   const char *mError;
public:
   SQLiteCantOpenEvent(S32 errorCode = -1, const char *errorMessage = NULL)
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
   ~SQLiteCantOpenEvent()
   {
      SAFE_FREE(mError);
   }
   virtual void process( SimObject *object )
   {
      SQLiteInterface *sqli = dynamic_cast<SQLiteInterface*>(object);
      if(sqli)
      {
         if(sqli->isMethod("onOpenFailed"))
            Con::executef(sqli, 3, "onOpenFailed", avar("%d", mErrorCode), mError);
         else
            Con::errorf("SQLiteInterface [%d] (%s) error: can't open database!", sqli->getId(), sqli->getName());
      }
      else
         Con::errorf("Error: SQLiteInterface object not found!");
   }
};

class SQLiteOpenEvent : public SimEvent
{
public:
   SQLiteOpenEvent() {}
   ~SQLiteOpenEvent() {}
   virtual void process( SimObject *object )
   {
      SQLiteInterface *sqli = dynamic_cast<SQLiteInterface*>(object);
      if(sqli)
      {
         if(sqli->isMethod("onInitialized"))
            Con::executef(sqli, 1, "onInitialized");
         else
            Con::printf("SQLiteInterface [%d] (%s) initialized", sqli->getId(), sqli->getName());
      }
      else
         Con::errorf("Error: SQLiteInterface object not found!");
   }
};

class SQLiteGeneralErrorEvent : public SimEvent
{
   S32 mErrorCode;
   const char *mError;
public:
   SQLiteGeneralErrorEvent(S32 errorCode = -1, const char *errorMessage = NULL)
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
   ~SQLiteGeneralErrorEvent()
   {
      SAFE_FREE(mError);
   }
   virtual void process( SimObject *object )
   {
      SQLiteInterface *sqli = dynamic_cast<SQLiteInterface*>(object);
      if(sqli)
      {
         if(sqli->isMethod("onError"))
            Con::executef(sqli, 3, "onError", avar("%d", mErrorCode), mError);
         else
            Con::errorf("SQLiteInterface [%d] (%s) error!", sqli->getId(), sqli->getName());
      }
      else
         Con::errorf("Error: SQLiteInterface object not found!");
   }
};

void SQLiteInterface::_dumpError( S32 errorCode )
{
   char* errorMsg = (char *)sqlite3_errmsg(mSQLiteDB);
   Sim::postEvent(this, new SQLiteGeneralErrorEvent(errorCode, errorMsg), Sim::getTargetTime());
   sqlite3_free(errorMsg);
}

S32 SQLiteInterface::_openSQLiteDB()
{
   S32 result = sqlite3_open(mDatabase, &mSQLiteDB);
   if(result != SQLITE_OK)
   {
      char* errorMsg = (char *)sqlite3_errmsg(mSQLiteDB);
      Sim::postEvent(this, new SQLiteCantOpenEvent(result, errorMsg), Sim::getTargetTime());
      sqlite3_free(errorMsg);
   }
   else
   {
      setInitialized(true);
      Sim::postEvent(this, new SQLiteOpenEvent(), Sim::getTargetTime());
   }
   return result;
}

void SQLiteInterface::postSQLEvent( SQLiteRequest &event )
{
   if(!isInitialized())
   {
      Con::errorf("Can't post SQLiteRequest as the SQLiteInterface [%d] is not initialized!", this->getId());
      return;
   }
   if(isShuttingDown())
   {
      Con::errorf("Can't post SQLiteRequest as the SQLiteInterface [%d] is shutting down!", this->getId());
      return;
   }
   if(isDebug())
      Con::errorf("SQLiteRequest::postSQLEvent: %d", S32(event.type));
   Mutex::lockMutex(m_SQLiteMutex);
   // Create a deep copy of event, and save a pointer to the copy in a vector.
   SQLiteRequest* copy = (SQLiteRequest*)dMalloc(event.size);
   dMemcpy(copy, &event, event.size);
   mp_CurrentEventQueue->push_back(copy);
   Mutex::unlockMutex(m_SQLiteMutex);
   // Trigger the semaphore, so the thread can process the event
   m_ActionSemaphore->release();
}

void SQLiteInterface::processLoop()
{
   while(!mv_ShuttingDown)
   {
      // Wait for the event to be signaled, so we don't run for nothing
      m_ActionSemaphore->acquire();

      Mutex::lockMutex(m_SQLiteMutex);

      // swap event queue pointers
      Vector<SQLiteRequest*> &fullEventQueue = *mp_CurrentEventQueue;
      if(mp_CurrentEventQueue == &m_SQLiteEventQueue1)
         mp_CurrentEventQueue = &m_SQLiteEventQueue2;
      else
         mp_CurrentEventQueue = &m_SQLiteEventQueue1;

      if(isDebug())
         Con::printf("SQLiteInterface::processLoop -- processing %d elements from the %s queue.", (mp_CurrentEventQueue == &m_SQLiteEventQueue1) ? "first" : "second");

      Mutex::unlockMutex(m_SQLiteMutex);

      // Keep track of the original size
      const int size = fullEventQueue.size();
      // Walk the event queue in fifo order, processing the events, then clear the queue.
      for(S32 i=0; i < size; i++)
      {
         Mutex::lockMutex(m_SQLiteExclusiveMutex);
         S32 result = processSQLiteEvent(fullEventQueue[i]);
         if(result != SQLITE_OK)
            _dumpError(result);
         SAFE_FREE(fullEventQueue[i]);
         Mutex::unlockMutex(m_SQLiteExclusiveMutex);
      }
      fullEventQueue.clear();
   }
}

void SQLiteInterface::processThread( void *udata )
{
   SQLiteInterface* pThis = (SQLiteInterface *)udata;
   const S32 result = pThis->_openSQLiteDB();
   if(pThis->isDebug())
      Con::printf("SQLiteInterface [%d] (%s) initialization status: %d", pThis->getId(), pThis->getName(), result);
   if(result != SQLITE_OK)
   {
      pThis->_dumpError(result);
      return;
   }
   pThis->processLoop();
}

void SQLiteInterface::initialize( const char *database )
{
   if(isInitialized())
   {
      Con::errorf("SQLiteInterface::initialize() error: SQLiteInterface is already initialized!");
      return;
   }
   if(isShuttingDown())
   {
      Con::errorf("SQLiteInterface::initialize() error: SQLiteInterface [%d] is shutting down!", this->getId());
      return;
   }
   // Sanity check
   if(!Platform::isFile(database))
   {
      Con::errorf("SQLiteInterface::initialize() failed: can't find %s", database);
      return;
   }

   // Save the path to the database file
   mDatabase = (char*)dMalloc(dStrlen(database)+1);
   dMemset((void*)mDatabase, 0, dStrlen(database)+1);
   dMemcpy((void*)mDatabase, (void*)database, dStrlen(database));

   // Create a new thread
   m_SQLiteProcessThread = new Thread((ThreadRunFunction)processThread, this);
   // Auto-delete thread when finished.
   m_SQLiteProcessThread->autoDelete = true;
   // Ready, steady, go!
   m_SQLiteProcessThread->start();
}

/*
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