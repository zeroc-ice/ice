// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef DATABASE_CACHE_H
#define DATABASE_CACHE_H

#include <Ice/Plugin.h>
#include <IceDB/IceDB.h>
#include <IceStorm/LLURecord.h>
#include <IceStorm/SubscriberRecord.h>

namespace IceStorm
{

// Forward declarations
class Instance;
typedef IceUtil::Handle<Instance> InstancePtr;

class LLUWrapper : public IceUtil::Shared
{
public:

    virtual void put(const IceStormElection::LogUpdate&) = 0;
    virtual IceStormElection::LogUpdate get() = 0;
};
typedef IceUtil::Handle<LLUWrapper> LLUWrapperPtr;

class SubscribersWrapper : public virtual IceDB::Wrapper<SubscriberRecordKey, SubscriberRecord>
{
public:

    virtual void eraseTopic(const Ice::Identity&) = 0;
};
typedef IceUtil::Handle<SubscribersWrapper> SubscribersWrapperPtr;

class ConnectionPool : public virtual IceDB::ConnectionPool
{
public:

    virtual LLUWrapperPtr getLLU(const IceDB::DatabaseConnectionPtr&) = 0;
    virtual SubscribersWrapperPtr getSubscribers(const IceDB::DatabaseConnectionPtr&) = 0;
};
typedef IceUtil::Handle<ConnectionPool> ConnectionPoolPtr;

class DatabasePlugin : public Ice::Plugin
{
public:

    virtual ConnectionPoolPtr getConnectionPool(const std::string&) = 0;
};
typedef IceUtil::Handle<DatabasePlugin> DatabasePluginPtr;

}


#endif
