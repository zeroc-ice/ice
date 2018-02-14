// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceGrid/Internal.h>
#include <IceGrid/QueryI.h>
#include <IceGrid/Database.h>

using namespace std;
using namespace Ice;
using namespace IceGrid;

QueryI::QueryI(const CommunicatorPtr& communicator, const DatabasePtr& database) :
    _communicator(communicator),
    _database(database)
{
}

QueryI::~QueryI()
{
}

Ice::ObjectPrx
QueryI::findObjectById(const Ice::Identity& id, const Ice::Current&) const
{
    try
    {
        return _database->getObjectProxy(id);
    }
    catch(const ObjectNotRegisteredException&)
    {
        return 0;
    }
}

Ice::ObjectPrx
QueryI::findObjectByType(const string& type, const Ice::Current& current) const
{
    return _database->getObjectByType(type, current.con, current.ctx);
}

Ice::ObjectPrx
QueryI::findObjectByTypeOnLeastLoadedNode(const string& type, LoadSample sample, const Ice::Current& current) const
{
    return _database->getObjectByTypeOnLeastLoadedNode(type, sample, current.con, current.ctx);
}

Ice::ObjectProxySeq
QueryI::findAllObjectsByType(const string& type, const Ice::Current& current) const
{
    return _database->getObjectsByType(type, current.con, current.ctx);
}

Ice::ObjectProxySeq
QueryI::findAllReplicas(const Ice::ObjectPrx& proxy, const Ice::Current& current) const
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
    Ice::ObjectPrx prx = proxy;
    if(prx->ice_getAdapterId().empty())
    {
        try
        {
            ObjectInfo info = _database->getObjectInfo(prx->ice_getIdentity());
            prx = info.proxy;
        }
        catch(const ObjectNotRegisteredException&)
        {
            return Ice::ObjectProxySeq();
        }
    }

    try
    {
        AdapterInfoSeq infos = _database->getFilteredAdapterInfo(prx->ice_getAdapterId(), current.con, current.ctx);
        if(infos.empty() || infos[0].replicaGroupId != prx->ice_getAdapterId()) 
        {
            // The adapter id doesn't refer to a replica group or the replica group is empty.
            return Ice::ObjectProxySeq();
        }

        Ice::ObjectProxySeq proxies;
        for(AdapterInfoSeq::const_iterator p = infos.begin(); p != infos.end(); ++p)
        {
            assert(!p->id.empty());
            proxies.push_back(prx->ice_adapterId(p->id));
        }
        return proxies;
    }
    catch(const AdapterNotExistException&)
    {
        return Ice::ObjectProxySeq();
    }
}
