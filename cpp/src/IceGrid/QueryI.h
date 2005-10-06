// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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

    virtual ::Ice::ObjectPrx findObjectById(const ::Ice::Identity&, const ::Ice::Current&) const;
    virtual ::Ice::ObjectPrx findObjectByType(const ::std::string&, const ::Ice::Current&) const;
    virtual ::Ice::ObjectPrx findObjectByTypeOnLeastLoadedNode(const ::std::string&, LoadSample, 
							       const ::Ice::Current&) const;
    virtual ::Ice::ObjectProxySeq findAllObjectsWithType(const ::std::string&, const ::Ice::Current&) const;

private:

    Ice::CommunicatorPtr _communicator;
    DatabasePtr _database;
};

}

#endif
