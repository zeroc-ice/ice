// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef DATABASE_WRAPPER_H
#define DATABASE_WRAPPER_H

#include <IceStorm/DatabaseCache.h>
#ifndef QTSQL
#  include <IceStorm/LLUMap.h>
#  include <IceStorm/SubscriberMap.h>
#endif

namespace IceStorm
{

#ifdef QTSQL

class LLUWrapper
{
public:

    LLUWrapper(const DatabaseCachePtr& database, const IceSQL::DatabaseConnectionPtr& connection) :
        _table(database->llu),
        _connection(connection)
    {
    }

    void put(const IceStormElection::LogUpdate& llu)
    {
        _table->put(_connection, llu);
    }

    IceStormElection::LogUpdate get()
    {
        IceStormElection::LogUpdate llu;
        try
        {
            llu = _table->get(_connection);
        }
        catch(const IceSQL::NotFoundException&)
        {
            assert(false);
        }
        return llu;
    }

private:

    const SqlLLUPtr _table;
    const IceSQL::DatabaseConnectionPtr _connection;
};

class SubscriberMapWrapper
{
public:

    SubscriberMapWrapper(const DatabaseCachePtr& database, const IceSQL::DatabaseConnectionPtr& connection) :
        _table(database->subscriberMap),
        _connection(connection)
    {
    }

    SubscriberMap getMap()
    {
        SubscriberMap smap;
        _table->getMap(_connection, smap);
        return smap;
    }

    void
    put(const SubscriberRecordKey& key, const SubscriberRecord& rec)
    {
        _table->put(_connection, key, rec);
    }

    SubscriberRecord
    find(const SubscriberRecordKey& key)
    {
        return _table->find(_connection, key);
    }

    void
    erase(const SubscriberRecordKey& key)
    {
        _table->erase(_connection, key);
    }

    void
    eraseTopic(const Ice::Identity& topic)
    {
        _table->eraseTopic(_connection, topic);
    }

    void
    clear()
    {
        _table->clear(_connection);
    }

private:

    const SqlSubscriberMapPtr _table;
    const IceSQL::DatabaseConnectionPtr _connection;
};


#else

class NotFoundException : public std::exception
{
};

class LLUWrapper
{
public:

    LLUWrapper(const DatabaseCachePtr& database, const DatabaseConnectionPtr& connection) :
        _llumap(connection, "llu")
    {
    }

    void put(const IceStormElection::LogUpdate& llu)
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

    IceStormElection::LogUpdate get()
    {
        LLUMap::iterator ci = _llumap.find("_manager");
        assert(ci != _llumap.end());
        return ci->second;
    }

private:

    LLUMap _llumap;
};

class SubscriberMapWrapper
{
public:

    SubscriberMapWrapper(const DatabaseCachePtr& database, const DatabaseConnectionPtr& connection) :
        _subscriberMap(connection, "subscribers")
    {
    }

    SubscriberMap& getMap()
    {
        return _subscriberMap;
    }

    void
    put(const SubscriberRecordKey& key, const SubscriberRecord& rec)
    {
        _subscriberMap.put(SubscriberMap::value_type(key, rec));
    }

    SubscriberRecord
    find(const SubscriberRecordKey& key)
    {
        SubscriberMap::const_iterator q = _subscriberMap.find(key);
        if(q == _subscriberMap.end())
        {
            throw NotFoundException();
        }
        return q->second;
    }

    void
    erase(const SubscriberRecordKey& key)
    {
        _subscriberMap.erase(key);
    }

    void
    eraseTopic(const Ice::Identity& topic)
    {
        SubscriberRecordKey key;
        key.topic = topic;

        SubscriberMap::iterator p = _subscriberMap.find(key);
        while(p != _subscriberMap.end() && p->first.topic == key.topic)
        {
            _subscriberMap.erase(p++);
        }
    }

    void
    clear()
    {
        _subscriberMap.clear();
    }

private:

    SubscriberMap _subscriberMap;
};

#endif

}

#endif
