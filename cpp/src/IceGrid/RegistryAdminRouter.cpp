// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceGrid/RegistryAdminRouter.h>
#include <Ice/Ice.h>

using namespace IceGrid;
using namespace Ice;
using namespace std;


IceGrid::RegistryServerAdminRouter::RegistryServerAdminRouter(const DatabasePtr& database) :
    _database(database)
{
}

ObjectPrx
IceGrid::RegistryServerAdminRouter::getTarget(const Current& current)
{
    ObjectPrx target = 0;

    try
    {
        target = _database->getServer(current.id.name)->getAdminProxy();
    }
    catch(const ServerNotExistException&)
    {
    }
    catch(const NodeUnreachableException&)
    {
    }
    catch(const DeploymentException&)
    {
    }

    if(target == 0)
    {
        throw ObjectNotExistException(__FILE__, __LINE__);
    }
    
    return target->ice_facet(current.facet);
}


IceGrid::RegistryNodeAdminRouter::RegistryNodeAdminRouter(const string& collocNodeName, const DatabasePtr& database) :
    _collocNodeName(collocNodeName),
    _database(database)
{
}

ObjectPrx
IceGrid::RegistryNodeAdminRouter::getTarget(const Current& current)
{
    ObjectPrx target;

    if(!_collocNodeName.empty() && current.id.name == _collocNodeName)
    {
        // Straight to the local Admin object
        target = current.adapter->getCommunicator()->getAdmin();
    }
    else
    {
        try
        {
            target = _database->getNode(current.id.name)->getAdminProxy();
        }
        catch(const NodeUnreachableException&)
        {
        }
        catch(const NodeNotExistException&)
        {
        }
        
        if(target == 0)
        {
            throw ObjectNotExistException(__FILE__, __LINE__);
        }
    }

    return target->ice_facet(current.facet);
}



IceGrid::RegistryReplicaAdminRouter::RegistryReplicaAdminRouter(const string& name, 
                                                                const DatabasePtr& database) :
    _name(name),
    _database(database)
{
}

ObjectPrx
IceGrid::RegistryReplicaAdminRouter::getTarget(const Current& current)
{
    ObjectPrx target;

    if(current.id.name == _name)
    {
        // Straight to the local Admin object
        target = current.adapter->getCommunicator()->getAdmin();
    }
    else
    {
        try
        {
            // Forward to Admin object in remote replica
            target = _database->getReplica(current.id.name)->getAdminProxy();
        }
        catch(const RegistryNotExistException&)
        {
        }
    }

    if(target == 0)
    {
        throw ObjectNotExistException(__FILE__, __LINE__);
    }
    
    return target->ice_facet(current.facet);    
}
