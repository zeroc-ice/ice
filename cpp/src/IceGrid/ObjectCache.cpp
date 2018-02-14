// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Random.h>
#include <Ice/Communicator.h>
#include <Ice/LoggerUtil.h>
#include <Ice/LocalException.h>
#include <IceGrid/ObjectCache.h>
#include <IceGrid/NodeSessionI.h>
#include <IceGrid/ServerCache.h>
#include <IceGrid/SessionI.h>

using namespace std;
using namespace IceGrid;

pointer_to_unary_function<int, unsigned int> ObjectCache::_rand(IceUtilInternal::random);

namespace IceGrid
{

struct ObjectEntryCI : binary_function<ObjectEntryPtr&, ObjectEntryPtr&, bool>
{

    bool
    operator()(const ObjectEntryPtr& lhs, const ObjectEntryPtr& rhs)
    {
        return ::Ice::proxyIdentityLess(lhs->getProxy(), rhs->getProxy());
    }
};

struct ObjectLoadCI : binary_function<pair<Ice::ObjectPrx, float>&, pair<Ice::ObjectPrx, float>&, bool>
{
    bool operator()(const pair<Ice::ObjectPrx, float>& lhs, const pair<Ice::ObjectPrx, float>& rhs)
    {
        return lhs.second < rhs.second;
    }
};

};

ObjectCache::TypeEntry::TypeEntry()
{
}

void
ObjectCache::TypeEntry::add(const ObjectEntryPtr& obj)
{
    //
    // No mutex protection here, this is called with the cache locked.
    //
    _objects.insert(lower_bound(_objects.begin(), _objects.end(), obj, ObjectEntryCI()), obj);
}

bool
ObjectCache::TypeEntry::remove(const ObjectEntryPtr& obj)
{
    //
    // No mutex protection here, this is called with the cache locked.
    //
    vector<ObjectEntryPtr>::iterator q = lower_bound(_objects.begin(), _objects.end(), obj, ObjectEntryCI());
    assert(q->get() == obj.get());
    _objects.erase(q);
    return _objects.empty();
}

ObjectCache::ObjectCache(const Ice::CommunicatorPtr& communicator) : _communicator(communicator)
{
}

void
ObjectCache::add(const ObjectInfo& info, const string& application, const string& server)
{
    const Ice::Identity& id = info.proxy->ice_getIdentity();

    Lock sync(*this);
    if(getImpl(id))
    {
        Ice::Error out(_communicator->getLogger());
        out << "can't add duplicate object `" << _communicator->identityToString(id) << "'";
        return;
    }

    ObjectEntryPtr entry = new ObjectEntry(info, application, server);
    addImpl(id, entry);

    map<string, TypeEntry>::iterator p = _types.find(entry->getType());
    if(p == _types.end())
    {
        p = _types.insert(p, map<string, TypeEntry>::value_type(entry->getType(), TypeEntry()));
    }
    p->second.add(entry);

    if(_traceLevels && _traceLevels->object > 0)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->objectCat);
        out << "added object `" << _communicator->identityToString(id) << "'";
    }
}

ObjectEntryPtr
ObjectCache::get(const Ice::Identity& id) const
{
    Lock sync(*this);
    ObjectEntryPtr entry = getImpl(id);
    if(!entry)
    {
        throw ObjectNotRegisteredException(id);
    }
    return entry;
}

void
ObjectCache::remove(const Ice::Identity& id)
{
    Lock sync(*this);
    ObjectEntryPtr entry = getImpl(id);
    if(!entry)
    {
        Ice::Error out(_communicator->getLogger());
        out << "can't remove unknown object `" << _communicator->identityToString(id) << "'";
        return;
    }
    removeImpl(id);

    map<string, TypeEntry>::iterator p = _types.find(entry->getType());
    assert(p != _types.end());
    if(p->second.remove(entry))
    {
        _types.erase(p);
    }

    if(_traceLevels && _traceLevels->object > 0)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->objectCat);
        out << "removed object `" << _communicator->identityToString(id) << "'";
    }
}

vector<ObjectEntryPtr>
ObjectCache::getObjectsByType(const string& type)
{
    Lock sync(*this);
    map<string, TypeEntry>::const_iterator p = _types.find(type);
    if(p == _types.end())
    {
        return vector<ObjectEntryPtr>();
    }
    return p->second.getObjects();
}

ObjectInfoSeq
ObjectCache::getAll(const string& expression)
{
    Lock sync(*this);
    ObjectInfoSeq infos;
    for(map<Ice::Identity, ObjectEntryPtr>::const_iterator p = _entries.begin(); p != _entries.end(); ++p)
    {
        if(expression.empty() || IceUtilInternal::match(_communicator->identityToString(p->first), expression, true))
        {
            infos.push_back(p->second->getObjectInfo());
        }
    }
    return infos;
}

ObjectInfoSeq
ObjectCache::getAllByType(const string& type)
{
    Lock sync(*this);
    ObjectInfoSeq infos;
    map<string, TypeEntry>::const_iterator p = _types.find(type);
    if(p == _types.end())
    {
        return infos;
    }

    const vector<ObjectEntryPtr>& objects = p->second.getObjects();
    for(vector<ObjectEntryPtr>::const_iterator q = objects.begin(); q != objects.end(); ++q)
    {
        infos.push_back((*q)->getObjectInfo());
    }
    return infos;
}

ObjectEntry::ObjectEntry(const ObjectInfo& info, const string& application, const string& server) :
    _info(info),
    _application(application),
    _server(server)
{
}

Ice::ObjectPrx
ObjectEntry::getProxy() const
{
    return _info.proxy;
}

string
ObjectEntry::getType() const
{
    return _info.type;
}

string
ObjectEntry::getApplication() const
{
    return _application;
}

string
ObjectEntry::getServer() const
{
    return _server;
}

const ObjectInfo&
ObjectEntry::getObjectInfo() const
{
    return _info;
}

bool
ObjectEntry::canRemove()
{
    return true;
}
