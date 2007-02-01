// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
QueryI::findObjectByType(const string& type, const Ice::Current&) const
{
    return _database->getObjectByType(type);
}

Ice::ObjectPrx
QueryI::findObjectByTypeOnLeastLoadedNode(const string& type, LoadSample sample, const Ice::Current&) const
{
    return _database->getObjectByTypeOnLeastLoadedNode(type, sample);
}

Ice::ObjectProxySeq
QueryI::findAllObjectsByType(const string& type, const Ice::Current&) const
{
    return _database->getObjectsByType(type);
}


Ice::ObjectProxySeq
QueryI::findAllReplicas(const Ice::ObjectPrx& proxy, const Ice::Current&) const
{
    try
    {
        if(!proxy)
        {
            return Ice::ObjectProxySeq();
        }

        AdapterInfoSeq infos = _database->getAdapterInfo(proxy->ice_getAdapterId());
        assert(!infos.empty());
        if(infos[0].replicaGroupId != proxy->ice_getAdapterId()) // The adapter id doesn't refer to a replica group.
        {
            return Ice::ObjectProxySeq();
        }

        Ice::ObjectProxySeq proxies;
        for(AdapterInfoSeq::const_iterator p = infos.begin(); p != infos.end(); ++p)
        {
            assert(!p->id.empty());
            proxies.push_back(proxy->ice_adapterId(p->id));
        }
        return proxies;
    }
    catch(const AdapterNotExistException&)
    {
        return Ice::ObjectProxySeq();
    }
}
