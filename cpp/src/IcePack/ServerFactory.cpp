// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceUtil/UUID.h>
#include <Freeze/DB.h>
#include <Freeze/Evictor.h>
#include <IcePack/ServerFactory.h>
#include <IcePack/ServerI.h>
#include <IcePack/ServerAdapterI.h>
#include <IcePack/TraceLevels.h>

#include <map>

using namespace std;
using namespace IcePack;

namespace IcePack
{

class ServerFactoryServantLocator : public Ice::ServantLocator
{
};


class ServerFactoryServantInitializer : public Freeze::ServantInitializer
{
public:

    virtual void 
    initialize(const Ice::ObjectAdapterPtr& adapter, const Ice::Identity& identity, const Ice::ObjectPtr& servant)
    {
	//
	// Add the servant to the adapter active object map. This will
	// prevent the evictor from evicting the servant. We just use
	// the evictor to load servants from the database.
	//
	// TODO: Implement our own servant locator instead of using
	// the evictor.
	//
	adapter->add(servant, identity);
    }

};

};

IcePack::ServerFactory::ServerFactory(const Ice::ObjectAdapterPtr& adapter, 
				      const TraceLevelsPtr& traceLevels, 
				      const Freeze::DBEnvironmentPtr& dbEnv,
				      const ActivatorPtr& activator) :
    _adapter(adapter),
    _traceLevels(traceLevels),
    _activator(activator)
{
    Ice::PropertiesPtr properties = _adapter->getCommunicator()->getProperties();
    _waitTime = properties->getPropertyAsIntWithDefault("IcePack.Node.WaitTime", 60);
    
    Freeze::ServantInitializerPtr initializer = new ServerFactoryServantInitializer();

    //
    // Create and install the freeze evictor for server objects.
    //
    Freeze::DBPtr dbServers = dbEnv->openDB("servers", true);
    _serverEvictor = dbServers->createEvictor(Freeze::SaveUponEviction);
    _serverEvictor->setSize(10000);
    _serverEvictor->installServantInitializer(initializer);

    //
    // Create and install the freeze evictor for server adapter objects.
    //
    Freeze::DBPtr dbServerAdapters = dbEnv->openDB("serveradapters", true);
    _serverAdapterEvictor = dbServerAdapters->createEvictor(Freeze::SaveUponEviction);
    _serverAdapterEvictor->setSize(10000);
    _serverAdapterEvictor->installServantInitializer(initializer);

    //
    // Install the server object factory.
    // 
    _adapter->getCommunicator()->addObjectFactory(this, "::IcePack::Server");
    _adapter->getCommunicator()->addObjectFactory(this, "::IcePack::ServerAdapter");

    //
    // Install the evictors.
    //
    _adapter->addServantLocator(_serverEvictor, "IcePackServer");
    _adapter->addServantLocator(_serverAdapterEvictor, "IcePackServerAdapter");
}

//
// Ice::ObjectFactory::create method implementation
//
Ice::ObjectPtr
IcePack::ServerFactory::create(const string& type)
{
    if(type == "::IcePack::Server")
    {
	return new ServerI(this, _traceLevels, _activator, _waitTime);
    }
    else if(type == "::IcePack::ServerAdapter")
    {
	return new ServerAdapterI(this, _traceLevels, _waitTime);
    }
    else
    {
	assert(false);
    }
}

//
// Ice::ObjectFactory::destroy method implementation
//
void 
IcePack::ServerFactory::destroy()
{
    _adapter = 0;
    _serverEvictor = 0;
    _serverAdapterEvictor = 0;
    _traceLevels = 0;
    _activator = 0;
}

//
// Create a new server servant and new server adapter servants from
// the given description.
//
ServerPrx
IcePack::ServerFactory::createServerAndAdapters(const ServerDescription& description, 
						const vector<string>& adapterNames,
						map<string, ServerAdapterPrx>& adapters)
{
    //
    // Create the server object.
    //
    ServerPtr serverI = new ServerI(this, _traceLevels, _activator, _waitTime);

    serverI->description = description;

    Ice::Identity id;
    id.category = "IcePackServer";
    id.name = description.name + "-" + IceUtil::generateUUID();

    //
    // Create the server adapters.
    //
    ServerPrx proxy = ServerPrx::uncheckedCast(_adapter->createProxy(id));
    for(Ice::StringSeq::const_iterator p = adapterNames.begin(); p != adapterNames.end(); ++p)
    {
	ServerAdapterPrx adapterProxy = createServerAdapter(*p, proxy);	
	adapters[*p] = adapterProxy;
	serverI->adapters.push_back(adapterProxy);
    }

    //
    // By default server is always activated on demand.
    //
    serverI->activation = OnDemand;

    _adapter->add(serverI, id);
    
    _serverEvictor->createObject(id, serverI);

    if(_traceLevels->server > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->serverCat);
	out << "created server `" << description.name << "'";
    }
    
    return proxy;
}

//
// Create a new server adapter servant with the given name and server
// and add it the evictor database.
//
ServerAdapterPrx
IcePack::ServerFactory::createServerAdapter(const string& name, const ServerPrx& server)
{
    ServerAdapterPtr adapterI = new ServerAdapterI(this, _traceLevels, _waitTime);
    adapterI->name = name;
    adapterI->theServer = server;

    Ice::Identity id;
    id.category = "IcePackServerAdapter";
    id.name = name + "-" + IceUtil::generateUUID();

    _adapter->add(adapterI, id);
    
    _serverAdapterEvictor->createObject(id, adapterI);

    if(_traceLevels->adapter > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->adapterCat);
	out << "created server adapter `" << name << "'";
    }
    
    return ServerAdapterPrx::uncheckedCast(_adapter->createProxy(id));
}

void
IcePack::ServerFactory::destroy(const ServerPtr& server, const Ice::Identity& identity)
{
    try
    {
	_serverEvictor->destroyObject(identity);

	if(_traceLevels->server > 0)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->serverCat);
	    out << "destroyed server `" << server->description.name << "'";
	}
    }
    catch(const Freeze::DBException& ex)
    {
	ostringstream os;
	os << "couldn't destroy server `" << server->description.name << "':\n" << ex;
	_traceLevels->logger->warning(os.str());
    }
    catch(const Freeze::EvictorDeactivatedException&)
    {
	assert(false);
    }

    _adapter->remove(identity);
}

void
IcePack::ServerFactory::destroy(const ServerAdapterPtr& adapter, const Ice::Identity& identity)
{
    try
    {
	_serverAdapterEvictor->destroyObject(identity);

	if(_traceLevels->adapter > 0)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->adapterCat);
	    out << "destroyed server adapter `" << adapter->name << "'";
	}
    }
    catch(const Freeze::DBException& ex)
    {
	ostringstream os;
	os << "couldn't destroy server adapter `" << adapter->name << "':\n" << ex;
	_traceLevels->logger->warning(os.str());
    }
    catch(const Freeze::EvictorDeactivatedException&)
    {
	assert(false);
    }

    _adapter->remove(identity);
}
