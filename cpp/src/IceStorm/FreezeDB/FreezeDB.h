// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef FREEZE_DATABASE_CACHE_H
#define FREEZE_DATABASE_CACHE_H

#include <IceDB/FreezeTypes.h>

#include <IceStorm/DB.h>

namespace IceStorm
{

class FreezeConnectionPool : public FreezeDB::ConnectionPool, public ConnectionPool
{
public:

    FreezeConnectionPool(const Ice::CommunicatorPtr&, const std::string&);

    virtual LLUWrapperPtr getLLU(const IceDB::DatabaseConnectionPtr&);
    virtual SubscribersWrapperPtr getSubscribers(const IceDB::DatabaseConnectionPtr&);
};
typedef IceUtil::Handle<FreezeConnectionPool> FreezeConnectionPoolPtr;

class FreezeDBPlugin : public DatabasePlugin
{
public:

    FreezeDBPlugin(const Ice::CommunicatorPtr&);

    void initialize();
    void destroy();
    
    ConnectionPoolPtr getConnectionPool(const std::string&);

private:

    const Ice::CommunicatorPtr _communicator;
};

}

#endif
