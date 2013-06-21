//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
// Copyright Dedicated Logic 2011
//-----------------------------------------------------------------------------

#ifndef _SQLITE_EVENTS_H_
#define _SQLITE_EVENTS_H_

#ifndef _SIMBASE_H_
#include "console/simEvents.h"
#endif

#include "collection/hashTable.h"

enum SQLiteEventType
{
   StubEventType,
   GenericQuery,
   GenericSelectQuery,
   GenericUpdateQuery,
   ExampleQuery,
   ExclusiveQuery,
   // Feel free to add your own additional types
};

struct SQLiteRequest
{
   S32 mSubmitCounter;
   U16 size;
   SQLiteEventType type;
   SQLiteRequest() :
      mSubmitCounter(0)
   {
      size = sizeof(SQLiteRequest);
      type = StubEventType;
   }
};

class SQLiteRecordSet;

struct GenericSQLiteQueryRequest
   : public SQLiteRequest
{
   SQLiteRecordSet* mRecordSet;
   GenericSQLiteQueryRequest() :
      mRecordSet(NULL)
   {
      size = sizeof(GenericSQLiteQueryRequest);
      type = GenericQuery;
   }
};

struct ExclusiveSQLiteSelectQueryRequest
   : public GenericSQLiteQueryRequest
{
   ExclusiveSQLiteSelectQueryRequest()
   {
      size = sizeof(ExclusiveSQLiteSelectQueryRequest);
      type = ExclusiveQuery;
   }
};

// Example on how to use own requests
struct GenericSQLiteSelectQueryRequest : public GenericSQLiteQueryRequest
{
   GenericSQLiteSelectQueryRequest()
   {
      size = sizeof(GenericSQLiteSelectQueryRequest);
      type = GenericSelectQuery;
   }
};

struct GenericSQLiteUpdateQueryRequest : public GenericSQLiteQueryRequest
{
   GenericSQLiteUpdateQueryRequest()
   {
      size = sizeof(GenericSQLiteUpdateQueryRequest);
      type = GenericUpdateQuery;
   }
};

// SQLiExample
//class HashTable;
struct ExampleSQLiteQueryRequest : public SQLiteRequest
{
   HashMap<const char*, char*> *ao;
   ExampleSQLiteQueryRequest()
   {
      ao = NULL;
      size = sizeof(ExampleSQLiteQueryRequest);
      type = ExampleQuery;
   }
};

#endif // _SQLITE_EVENTS_H_
