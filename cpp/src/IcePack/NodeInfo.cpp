// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IcePack/NodeInfo.h>
#include <IcePack/ServerFactory.h>
#include <IcePack/TraceLevels.h>

using namespace std;
using namespace IcePack;

IcePack::NodeInfo::NodeInfo(const Ice::CommunicatorPtr& communicator,
			    const ServerFactoryPtr& serverFactory,
			    const NodePtr& node,
			    const TraceLevelsPtr& traceLevels) :
    _communicator(communicator),
    _serverFactory(serverFactory),
    _node(node),
    _traceLevels(traceLevels)
{
}

Ice::CommunicatorPtr
IcePack::NodeInfo::getCommunicator() const
{
    return _communicator;
}

ServerFactoryPtr
IcePack::NodeInfo::getServerFactory() const
{
    return _serverFactory;
}

TraceLevelsPtr
IcePack::NodeInfo::getTraceLevels() const
{
    return _traceLevels;
}

NodePtr
IcePack::NodeInfo::getNode() const
{
    return _node;
}

AdapterRegistryPrx
IcePack::NodeInfo::getAdapterRegistry() const
{
    try
    {
	return AdapterRegistryPrx::checkedCast(
	    _communicator->stringToProxy("IcePack/AdapterRegistry@IcePack.Registry.Internal"));
    }
    catch(const Ice::LocalException&)
    {
	return 0;
    }
}

ObjectRegistryPrx
IcePack::NodeInfo::getObjectRegistry() const
{
    try
    {
	return ObjectRegistryPrx::checkedCast(
	    _communicator->stringToProxy("IcePack/ObjectRegistry@IcePack.Registry.Internal"));
    }
    catch(const Ice::LocalException&)
    {
	return 0;
    }
}

ServerRegistryPrx
IcePack::NodeInfo::getServerRegistry() const
{
    try
    {
	return ServerRegistryPrx::checkedCast(
	    _communicator->stringToProxy("IcePack/ServerRegistry@IcePack.Registry.Internal"));
    }
    catch(const Ice::LocalException&)
    {
	return 0;
    }
}
