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

#include <IceStorm/DB.h>

namespace IceStorm
{

class FreezeDatabaseCache : public FreezeDB::DatabaseCache, public DatabaseCache
{
public:

    FreezeDatabaseCache(const Ice::CommunicatorPtr&, const std::string&);

    virtual LLUWrapperPtr getLLU(const IceDB::DatabaseConnectionPtr&);
    virtual SubscribersWrapperPtr getSubscribers(const IceDB::DatabaseConnectionPtr&);
};
typedef IceUtil::Handle<FreezeDatabaseCache> FreezeDatabaseCachePtr;

class FreezeDBPlugin : public DatabasePlugin
{
public:

    FreezeDBPlugin(const Ice::CommunicatorPtr&);

    void initialize();
    void destroy();
    
    DatabaseCachePtr getDatabaseCache(const std::string&);

private:

    const Ice::CommunicatorPtr _communicator;
};

}
