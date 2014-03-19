// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceStorm/FreezeDB/FreezeDB.h>

#include <IceStorm/FreezeDB/LLUMap.h>
#include <IceStorm/FreezeDB/SubscriberMap.h>

#include <sys/types.h>
#include <sys/stat.h>

using namespace IceStorm;
using namespace std;

extern "C"
{

ICE_DECLSPEC_EXPORT ::Ice::Plugin*
createFreezeDB(const Ice::CommunicatorPtr& communicator, const string& /*name*/, const Ice::StringSeq& /*args*/)
{
    return new IceStorm::FreezeDBPlugin(communicator);
}

}

namespace
{

//
// Freeze wrappers for Freeze dictionaries
//

class FreezeLLUWrapper : public LLUWrapper
{
public:

    FreezeLLUWrapper(const Freeze::ConnectionPtr& connection, const std::string& dbName) :
        _llumap(connection, dbName)
    {
    }

    virtual void 
    put(const IceStormElection::LogUpdate& llu)
    {
        try
        {
            LLUMap::iterator ci = _llumap.find("_manager");
            if(ci == _llumap.end())
            {
                _llumap.put(LLUMap::value_type("_manager", llu));
            }
            else
            {
                ci.set(llu);
            }
        }
        catch(const Freeze::DatabaseException& ex)
        {
            FreezeDB::throwDatabaseException(__FILE__, __LINE__, ex);
        }
    }

    virtual IceStormElection::LogUpdate
    get()
    {
        try
        {
            LLUMap::iterator ci = _llumap.find("_manager");
            assert(ci != _llumap.end());
            return ci->second;
        }
        catch(const Freeze::DatabaseException& ex)
        {
            FreezeDB::throwDatabaseException(__FILE__, __LINE__, ex);
            return IceStormElection::LogUpdate(); // Keep the compiler happy.
        }
    }

private:

    LLUMap _llumap;
};

class FreezeSubscribersWrapper : public FreezeDB::Wrapper<SubscriberMap, SubscriberRecordKey, SubscriberRecord>,
                                 public SubscribersWrapper
{
public:

    FreezeSubscribersWrapper(const Freeze::ConnectionPtr& connection, const std::string& dbName) :
        FreezeDB::Wrapper<SubscriberMap, SubscriberRecordKey, SubscriberRecord>(connection, dbName)
    {
    }

    void
    eraseTopic(const Ice::Identity& topic)
    {
        try
        {
            SubscriberRecordKey key;
            key.topic = topic;
            SubscriberMap::iterator p = _dict.find(key);
            while(p != _dict.end() && p->first.topic == key.topic)
            {
                _dict.erase(p++);
            }
        }
        catch(const Freeze::DatabaseException& ex)
        {
            FreezeDB::throwDatabaseException(__FILE__, __LINE__, ex);
        }
    }
};

}

FreezeConnectionPool::FreezeConnectionPool(const Ice::CommunicatorPtr& communicator, const string& envName) :
    FreezeDB::ConnectionPool(communicator, envName)
{
}

LLUWrapperPtr
FreezeConnectionPool::getLLU(const IceDB::DatabaseConnectionPtr& connection)
{
    FreezeDB::DatabaseConnection* c = dynamic_cast<FreezeDB::DatabaseConnection*>(connection.get());
    return new FreezeLLUWrapper(c->freezeConnection(), "llu");
}

SubscribersWrapperPtr
FreezeConnectionPool::getSubscribers(const IceDB::DatabaseConnectionPtr& connection)
{
    FreezeDB::DatabaseConnection* c = dynamic_cast<FreezeDB::DatabaseConnection*>(connection.get());
    // COMPILERFIX: GCC 4.4 w/ -O2 emits strict aliasing warnings
    // without the follow temporary.
    SubscribersWrapper* w = new FreezeSubscribersWrapper(c->freezeConnection(), "subscribers");
    return w;
}

FreezeDBPlugin::FreezeDBPlugin(const Ice::CommunicatorPtr& communicator) : _communicator(communicator)
{
}

void
FreezeDBPlugin::initialize()
{
}

void
FreezeDBPlugin::destroy()
{
}

ConnectionPoolPtr
FreezeDBPlugin::getConnectionPool(const string& name)
{
    return new FreezeConnectionPool(_communicator, name);
}
