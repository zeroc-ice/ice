// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_QUERY_I_H
#define ICE_GRID_QUERY_I_H

#include <IceGrid/Query.h>
#include <Ice/CommunicatorF.h>

namespace IceGrid
{

class Database;
typedef IceUtil::Handle<Database> DatabasePtr;

class QueryI : public Query, public IceUtil::Mutex
{
public:

    QueryI(const Ice::CommunicatorPtr&, const DatabasePtr&);
    virtual ~QueryI();

    virtual void findObjectById_async(const AMD_Query_findObjectByIdPtr&, const ::Ice::Identity&, 
				      const ::Ice::Current&) const;

    virtual void findObjectByType_async(const AMD_Query_findObjectByTypePtr&, const ::std::string&, 
					const ::Ice::Current&) const;

    virtual void findObjectByTypeOnLeastLoadedNode_async(const AMD_Query_findObjectByTypeOnLeastLoadedNodePtr&, 
							 const ::std::string&, LoadSample, 
							 const ::Ice::Current&) const;

    virtual void findAllObjectsByType_async(const AMD_Query_findAllObjectsByTypePtr&, const ::std::string&, 
					    const ::Ice::Current&) const;

private:

    const Ice::CommunicatorPtr _communicator;
    const DatabasePtr _database;
};

}

#endif
