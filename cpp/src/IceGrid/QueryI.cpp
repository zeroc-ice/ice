//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceGrid/Internal.h>
#include <IceGrid/QueryI.h>
#include <IceGrid/Database.h>

using namespace std;
using namespace Ice;
using namespace IceGrid;

QueryI::QueryI(const shared_ptr<Communicator>& communicator, const shared_ptr<Database>& database) :
    _communicator(communicator),
    _database(database)
{
}

shared_ptr<Ice::ObjectPrx>
QueryI::findObjectById(Ice::Identity id, const Ice::Current&) const
{
    try
    {
        return _database->getObjectProxy(id);
    }
    catch(const ObjectNotRegisteredException&)
    {
        return nullptr;
    }
}

shared_ptr<Ice::ObjectPrx>
QueryI::findObjectByType(string type, const Ice::Current& current) const
{
    return _database->getObjectByType(move(type), current.con, current.ctx);
}

shared_ptr<Ice::ObjectPrx>
QueryI::findObjectByTypeOnLeastLoadedNode(string type, LoadSample sample, const Ice::Current& current) const
{
    return _database->getObjectByTypeOnLeastLoadedNode(move(type), move(sample), current.con, current.ctx);
}

Ice::ObjectProxySeq
QueryI::findAllObjectsByType(string type, const Ice::Current& current) const
{
    return _database->getObjectsByType(move(type), current.con, current.ctx);
}

Ice::ObjectProxySeq
QueryI::findAllReplicas(shared_ptr<Ice::ObjectPrx> proxy, const Ice::Current& current) const
{
    if(!proxy)
    {
        return Ice::ObjectProxySeq();
    }

    //
    // If the given proxy has an empty adapter id, we check if it's a
    // well-known object. If it's a well-known object we use the
    // registered proxy instead.
    //
    if(proxy->ice_getAdapterId().empty())
    {
        try
        {
            ObjectInfo info = _database->getObjectInfo(proxy->ice_getIdentity());
            proxy = info.proxy;
        }
        catch(const ObjectNotRegisteredException&)
        {
            return Ice::ObjectProxySeq();
        }
    }

    try
    {
        AdapterInfoSeq infos = _database->getFilteredAdapterInfo(proxy->ice_getAdapterId(), current.con, current.ctx);
        if(infos.empty() || infos[0].replicaGroupId != proxy->ice_getAdapterId())
        {
            // The adapter id doesn't refer to a replica group or the replica group is empty.
            return Ice::ObjectProxySeq();
        }

        Ice::ObjectProxySeq proxies;
        for(const auto& info : infos)
        {
            assert(!info.id.empty());
            proxies.push_back(proxy->ice_adapterId(info.id));
        }
        return proxies;
    }
    catch(const AdapterNotExistException&)
    {
        return Ice::ObjectProxySeq();
    }
}
