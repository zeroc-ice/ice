// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
    catch(const Ice::RuntimeException& ex)
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
    catch(const Ice::RuntimeException& ex)
    {
	return 0;
    }
}

Yellow::QueryPrx
IcePack::NodeInfo::getYellowQuery() const
{
    //
    // TODO: get it from the IcePack registry instead.
    //
    try
    {
	return Yellow::QueryPrx::checkedCast(_communicator->stringToProxy("Yellow/Query@Yellow.Query"));
    }
    catch(const Ice::RuntimeException& ex)
    {
	return 0;
    }
}

Yellow::AdminPrx
IcePack::NodeInfo::getYellowAdmin() const
{
    //
    // TODO: get it from the IcePack registry instead.
    //
    try
    {
	return Yellow::AdminPrx::checkedCast(_communicator->stringToProxy("Yellow/Admin@Yellow.Admin"));
    }
    catch(const Ice::RuntimeException& ex)
    {
	return 0;
    }
}

