// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

#include <IceDB/FreezeTypes.h>

#include <IceGrid/DB.h>

namespace IceGrid
{

class FreezeDatabaseCache : public FreezeDB::DatabaseCache, public DatabaseCache
{
public:

    FreezeDatabaseCache(const Ice::CommunicatorPtr&);

    virtual ApplicationsWrapperPtr getApplications(const IceDB::DatabaseConnectionPtr&);
    virtual AdaptersWrapperPtr getAdapters(const IceDB::DatabaseConnectionPtr&);
    virtual ObjectsWrapperPtr getObjects(const IceDB::DatabaseConnectionPtr&);
    virtual ObjectsWrapperPtr getInternalObjects(const IceDB::DatabaseConnectionPtr&);
};
typedef IceUtil::Handle<FreezeDatabaseCache> FreezeDatabaseCachePtr;

class FreezeDBPlugin : public DatabasePlugin
{
public:

    FreezeDBPlugin(const Ice::CommunicatorPtr&);

    void initialize();
    void destroy();
    
    DatabaseCachePtr getDatabaseCache();

private:

    const Ice::CommunicatorPtr _communicator;
    FreezeDatabaseCachePtr _databaseCache;
};

}
