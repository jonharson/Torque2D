//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
// Copyright Dedicated Logic 2011
//-----------------------------------------------------------------------------

#ifndef _SQLITE_EVENTS_H_
#define _SQLITE_EVENTS_H_

#include "SQLiteRecordSet.h"

namespace SQLite
{
	enum EventType
	{
	   StubEventType,
	   GenericQuery,
	   GenericSelectQuery,
	   GenericUpdateQuery,
	   DalObjectSelectQuery,
	   DalObjectUpdateQuery,
	   DalTableSelectQuery,
	   DalTableUpdateQuery,
	   ExclusiveQuery
	};

	struct Request
	{
	   S32 mSubmitCounter;
	   U16 size;
	   SQLite::EventType type;

	   Request(void) : mSubmitCounter(0)
	   {
		  size = sizeof(Request);
		  type = StubEventType;
	   }
	};

	struct GenericQueryRequest : public Request
	{
	   SQLite::RecordSet* mRecordSet;

	   GenericQueryRequest(void) : mRecordSet(NULL)
	   {
		  size = sizeof(GenericQueryRequest);
		  type = GenericQuery;
	   }
	};

	struct ExclusiveQueryRequest : public GenericQueryRequest
	{
	   ExclusiveQueryRequest(void)
	   {
		  size = sizeof(ExclusiveQueryRequest);
		  type = ExclusiveQuery;
	   }
	};

	struct GenericSelectQueryRequest : public GenericQueryRequest
	{
	   GenericSelectQueryRequest(void)
	   {
		  size = sizeof(GenericSelectQueryRequest);
		  type = GenericSelectQuery;
	   }
	};

	struct GenericUpdateQueryRequest : public GenericQueryRequest
	{
	   GenericUpdateQueryRequest(void)
	   {
		  size = sizeof(GenericUpdateQueryRequest);
		  type = GenericUpdateQuery;
	   }
	};

	struct DalObjectSelectQueryRequest : public GenericQueryRequest
	{
	   DalObjectSelectQueryRequest(void)
	   {
		  size = sizeof(DalObjectSelectQueryRequest);
		  type = DalObjectSelectQuery;
	   }
	};

	struct DalObjectUpdateQueryRequest : public GenericQueryRequest
	{
	   DalObjectUpdateQueryRequest(void)
	   {
		  size = sizeof(DalObjectUpdateQueryRequest);
		  type = DalObjectUpdateQuery;
	   }
	};

	struct DalTableSelectQueryRequest : public GenericQueryRequest
	{
	   DalTableSelectQueryRequest(void)
	   {
		  size = sizeof(DalTableSelectQueryRequest);
		  type = DalTableSelectQuery;
	   }
	};

	struct DalTableUpdateQueryRequest : public GenericQueryRequest
	{
	   DalTableUpdateQueryRequest(void)
	   {
		  size = sizeof(DalTableUpdateQueryRequest);
		  type = DalTableUpdateQuery;
	   }
	};
}

#endif // _SQLITE_EVENTS_H_
