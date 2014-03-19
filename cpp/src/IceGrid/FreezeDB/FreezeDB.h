// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef FREEZE_DATABASE_CACHE_H
#define FREEZE_DATABASE_CACHE_H

#include <IceDB/FreezeTypes.h>

#include <IceGrid/DB.h>

namespace IceGrid
{

class FreezeConnectionPool : public FreezeDB::ConnectionPool, public ConnectionPool
{
public:

    FreezeConnectionPool(const Ice::CommunicatorPtr&);

    virtual ApplicationsWrapperPtr getApplications(const IceDB::DatabaseConnectionPtr&);
    virtual AdaptersWrapperPtr getAdapters(const IceDB::DatabaseConnectionPtr&);
    virtual ObjectsWrapperPtr getObjects(const IceDB::DatabaseConnectionPtr&);
    virtual ObjectsWrapperPtr getInternalObjects(const IceDB::DatabaseConnectionPtr&);

    virtual std::map<std::string, Ice::Long> getSerials() const;
};
typedef IceUtil::Handle<FreezeConnectionPool> FreezeConnectionPoolPtr;

class FreezeDBPlugin : public DatabasePlugin
{
public:

    FreezeDBPlugin(const Ice::CommunicatorPtr&);

    void initialize();
    void destroy();
    
    bool initDB();
    ConnectionPoolPtr getConnectionPool();

private:

    const Ice::CommunicatorPtr _communicator;
    FreezeConnectionPoolPtr _connectionPool;
};

}

#endif
