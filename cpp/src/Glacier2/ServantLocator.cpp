// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/RoutingTable.h>
#include <Glacier2/ServantLocator.h>
#include <Glacier2/RouterI.h>

using namespace std;
using namespace Ice;
using namespace Glacier;

Glacier::ClientServantLocator::ClientServantLocator(const ObjectAdapterPtr& clientAdapter) :
    _communicator(clientAdapter->getCommunicator()),
    _properties(_communicator->getProperties()),
    _logger(_communicator->getLogger()),
    _clientAdapter(clientAdapter),
    _routerId(stringToIdentity(_properties->getPropertyWithDefault("Glacier2.Identity", "Glacier/router"))),
    _serverEndpoints(_properties->getProperty("Glacier2.Server.Endpoints")),
    _traceLevel(_properties->getPropertyAsInt("Glacier2.Trace.Session")),
    _serverAdapterCount(0),
    _clientMapHint(_clientMap.end())
{
}

ObjectPtr
Glacier::ClientServantLocator::locate(const Current& current, LocalObjectPtr&)
{
    IceUtil::Mutex::Lock sync(*this);

    assert(current.transport);
    
    map<TransportInfoPtr, Client>::iterator p = _clientMap.end();
    
    if(_clientMapHint != _clientMap.end() && _clientMapHint->first == current.transport)
    {
	p = _clientMapHint;
    }
    else
    {
	p = _clientMap.find(current.transport);

	if(p == _clientMap.end())
	{
	    //
	    // Create a server object adapter and related objects if
	    // server endpoints are defined.
	    //
	    ObjectAdapterPtr adapter;
	    CommunicatorPtr communicator = _clientAdapter->getCommunicator();
	    PropertiesPtr properties = communicator->getProperties();
	    string endpoints = properties->getProperty("Glacier2.Server.Endpoints");
	    if(!endpoints.empty())
	    {
		ostringstream name;
		name << "Glacier2.Server." << _serverAdapterCount++;
		adapter = communicator->createObjectAdapterWithEndpoints(name.str(), endpoints);
		ServerBlobjectPtr blobject = new ServerBlobject(communicator, current.transport);
		ServantLocatorPtr locator = new ServerServantLocator(blobject);
		adapter->addServantLocator(locator, "");
		adapter->activate();
	    }

	    //
	    // Add a new client to our client map.
	    //
	    Client client;
	    IceInternal::RoutingTablePtr routingTable = new IceInternal::RoutingTable;
	    client.router = new RouterI(_clientAdapter, adapter, routingTable);
	    client.clientBlobject = new ClientBlobject(_clientAdapter->getCommunicator(), routingTable, "");
	    client.serverAdapter = adapter;
	    p = _clientMap.insert(_clientMapHint, pair<const TransportInfoPtr, Client>(current.transport, client));

	    if(_traceLevel >= 1)
	    {
		Trace out(_logger, "Glacier");
		out << "added session for:\n";
		out << current.transport->toString();
		if(client.serverAdapter)
		{
		    Identity ident;
		    ident.name = "dummy";
		    string endpts = communicator->proxyToString(client.serverAdapter->createProxy(ident));
		    endpts.erase(0, endpts.find(':') + 1);
		    out << "\nserver adapter endpoints: " << endpts;
		}
	    }
	}
	
	_clientMapHint = p;
    }

    if(current.id == _routerId)
    {
	return p->second.router;
    }
    else
    {
	return p->second.clientBlobject;
    }
}

void
Glacier::ClientServantLocator::finished(const Current&, const ObjectPtr&, const LocalObjectPtr&)
{
    // Nothing to do
}

void
Glacier::ClientServantLocator::deactivate(const string&)
{
    IceUtil::Mutex::Lock sync(*this);

    for(map<TransportInfoPtr, Client>::iterator p = _clientMap.begin(); p != _clientMap.end(); ++p)
    {
	p->second.clientBlobject->destroy();
	RouterI* router = dynamic_cast<RouterI*>(p->second.router.get());
	router->destroy();
	if(p->second.serverAdapter)
	{
	    p->second.serverAdapter->deactivate();
	}
    };
    _clientMap.clear();
    _clientMapHint = _clientMap.end();
}

Glacier::ServerServantLocator::ServerServantLocator(const ServerBlobjectPtr& serverBlobject) :
    _serverBlobject(serverBlobject)
{
}

ObjectPtr
Glacier::ServerServantLocator::locate(const Current& current, LocalObjectPtr&)
{
    return _serverBlobject;
}

void
Glacier::ServerServantLocator::finished(const Current&, const ObjectPtr&, const LocalObjectPtr&)
{
    // Nothing to do
}

void
Glacier::ServerServantLocator::deactivate(const string&)
{
    _serverBlobject->destroy();
    _serverBlobject = 0;
}
