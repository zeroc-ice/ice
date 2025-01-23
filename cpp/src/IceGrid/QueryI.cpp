// Copyright (c) ZeroC, Inc.

#include "QueryI.h"
#include "Database.h"
#include "Internal.h"

using namespace std;
using namespace Ice;
using namespace IceGrid;

QueryI::QueryI(CommunicatorPtr communicator, const shared_ptr<Database>& database)
    : _communicator(std::move(communicator)),
      _database(database)
{
}

optional<Ice::ObjectPrx>
QueryI::findObjectById(Ice::Identity id, const Ice::Current&) const
{
    try
    {
        return _database->getObjectProxy(id);
    }
    catch (const ObjectNotRegisteredException&)
    {
        return nullopt;
    }
}

optional<Ice::ObjectPrx>
QueryI::findObjectByType(string type, const Ice::Current& current) const
{
    return _database->getObjectByType(type, current.con, current.ctx);
}

optional<Ice::ObjectPrx>
QueryI::findObjectByTypeOnLeastLoadedNode(string type, LoadSample sample, const Ice::Current& current) const
{
    return _database->getObjectByTypeOnLeastLoadedNode(type, sample, current.con, current.ctx);
}

Ice::ObjectProxySeq
QueryI::findAllObjectsByType(string type, const Ice::Current& current) const
{
    return _database->getObjectsByType(type, current.con, current.ctx);
}

Ice::ObjectProxySeq
QueryI::findAllReplicas(optional<Ice::ObjectPrx> proxy, const Ice::Current& current) const
{
    if (!proxy)
    {
        return {};
    }

    //
    // If the given proxy has an empty adapter id, we check if it's a
    // well-known object. If it's a well-known object we use the
    // registered proxy instead.
    //
    if (proxy->ice_getAdapterId().empty())
    {
        try
        {
            ObjectInfo info = _database->getObjectInfo(proxy->ice_getIdentity());
            proxy = info.proxy;
        }
        catch (const ObjectNotRegisteredException&)
        {
            return {};
        }
    }

    try
    {
        AdapterInfoSeq infos = _database->getFilteredAdapterInfo(proxy->ice_getAdapterId(), current.con, current.ctx);
        if (infos.empty() || infos[0].replicaGroupId != proxy->ice_getAdapterId())
        {
            // The adapter id doesn't refer to a replica group or the replica group is empty.
            return {};
        }

        Ice::ObjectProxySeq proxies;
        for (const auto& info : infos)
        {
            assert(!info.id.empty());
            proxies.emplace_back(proxy->ice_adapterId(info.id));
        }
        return proxies;
    }
    catch (const AdapterNotExistException&)
    {
        return {};
    }
}
