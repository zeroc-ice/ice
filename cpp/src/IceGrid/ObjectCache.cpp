// Copyright (c) ZeroC, Inc.

#include "ObjectCache.h"
#include "../Ice/Random.h"
#include "Ice/Communicator.h"
#include "Ice/LocalExceptions.h"
#include "Ice/LoggerUtil.h"
#include "NodeSessionI.h"
#include "ServerCache.h"
#include "SessionI.h"

using namespace std;
using namespace IceGrid;

namespace IceGrid
{
    bool compareObjectEntryCI(const shared_ptr<ObjectEntry>& lhs, const shared_ptr<ObjectEntry>& rhs)
    {
        return Ice::proxyIdentityLess(lhs->getProxy(), rhs->getProxy());
    }

    bool compareObjectLoadCI(
        const pair<optional<Ice::ObjectPrx>, float>& lhs,
        const pair<optional<Ice::ObjectPrx>, float>& rhs)
    {
        return lhs.second < rhs.second;
    }

};

void
ObjectCache::TypeEntry::add(const shared_ptr<ObjectEntry>& obj)
{
    // No mutex protection here, this is called with the cache locked.
    _objects.insert(lower_bound(_objects.begin(), _objects.end(), obj, compareObjectEntryCI), obj);
}

bool
ObjectCache::TypeEntry::remove(const shared_ptr<ObjectEntry>& obj)
{
    // No mutex protection here, this is called with the cache locked.
    auto q = lower_bound(_objects.begin(), _objects.end(), obj, compareObjectEntryCI);
    assert(q->get() == obj.get());
    _objects.erase(q);
    return _objects.empty();
}

ObjectCache::ObjectCache(const shared_ptr<Ice::Communicator>& communicator) : _communicator(communicator) {}

void
ObjectCache::add(const ObjectInfo& info, const string& application, const string& server)
{
    const Ice::Identity& id = info.proxy->ice_getIdentity();

    lock_guard lock(_mutex);
    if (getImpl(id))
    {
        Ice::Error out(_communicator->getLogger());
        out << "can't add duplicate object '" << _communicator->identityToString(id) << "'";
        return;
    }

    auto entry = make_shared<ObjectEntry>(info, application, server);
    addImpl(id, entry);

    auto p = _types.find(entry->getType());
    if (p == _types.end())
    {
        p = _types.insert(p, {entry->getType(), TypeEntry()});
    }
    p->second.add(entry);

    if (_traceLevels && _traceLevels->object > 0)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->objectCat);
        out << "added object '" << _communicator->identityToString(id) << "'";
    }
}

shared_ptr<ObjectEntry>
ObjectCache::get(const Ice::Identity& id) const
{
    lock_guard lock(_mutex);
    shared_ptr<ObjectEntry> entry = getImpl(id);
    if (!entry)
    {
        throw ObjectNotRegisteredException(id);
    }
    return entry;
}

void
ObjectCache::remove(const Ice::Identity& id)
{
    lock_guard lock(_mutex);
    shared_ptr<ObjectEntry> entry = getImpl(id);
    if (!entry)
    {
        Ice::Error out(_communicator->getLogger());
        out << "can't remove unknown object '" << _communicator->identityToString(id) << "'";
        return;
    }
    removeImpl(id);

    auto p = _types.find(entry->getType());
    assert(p != _types.end());
    if (p->second.remove(entry))
    {
        _types.erase(p);
    }

    if (_traceLevels && _traceLevels->object > 0)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->objectCat);
        out << "removed object '" << _communicator->identityToString(id) << "'";
    }
}

vector<shared_ptr<ObjectEntry>>
ObjectCache::getObjectsByType(const string& type)
{
    lock_guard lock(_mutex);
    auto p = _types.find(type);
    return p == _types.end() ? vector<shared_ptr<ObjectEntry>>{} : p->second.getObjects();
}

ObjectInfoSeq
ObjectCache::getAll(const string& expression)
{
    lock_guard lock(_mutex);
    ObjectInfoSeq infos;
    for (const auto& entry : _entries)
    {
        if (expression.empty() || IceInternal::match(_communicator->identityToString(entry.first), expression, true))
        {
            infos.push_back(entry.second->getObjectInfo());
        }
    }
    return infos;
}

ObjectInfoSeq
ObjectCache::getAllByType(const string& type)
{
    lock_guard lock(_mutex);
    ObjectInfoSeq infos;
    auto p = _types.find(type);
    if (p == _types.end())
    {
        return infos;
    }

    const vector<shared_ptr<ObjectEntry>>& objects = p->second.getObjects();
    for (const auto& object : objects)
    {
        infos.push_back(object->getObjectInfo());
    }
    return infos;
}

ObjectEntry::ObjectEntry(ObjectInfo info, string application, string server)
    : _info(std::move(info)),
      _application(std::move(application)),
      _server(std::move(server))
{
}

Ice::ObjectPrx
ObjectEntry::getProxy() const
{
    assert(_info.proxy);
    return *_info.proxy;
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
