// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/RoutingTable.h>
#include <Glacier2/RouterI.h>
#include <Glacier2/ServantLocator.h>

using namespace std;
using namespace Ice;
using namespace Glacier2;

Glacier2::ClientRouterI::ClientRouterI(const ObjectAdapterPtr& clientAdapter,
				       const ObjectAdapterPtr& serverAdapter,
				       const TransportInfoPtr& transport) :
    _logger(clientAdapter->getCommunicator()->getLogger()),
    _clientAdapter(clientAdapter),
    _serverAdapter(serverAdapter),
    _routingTable(new IceInternal::RoutingTable),
    _routingTableTraceLevel(clientAdapter->getCommunicator()->getProperties()->
			    getPropertyAsInt("Glacier2.Trace.RoutingTable")),
    _clientBlobject(new ClientBlobject(clientAdapter->getCommunicator(), _routingTable, "")),
    _serverBlobject(serverAdapter ? new ServerBlobject(serverAdapter->getCommunicator(), transport) : 0)
{
    if(_serverAdapter)
    {
	assert(_serverBlobject);
	ServantLocatorPtr serverLocator = new ServerServantLocator(_serverBlobject);
	_serverAdapter->addServantLocator(serverLocator, "");
	_serverAdapter->activate();
    }
}

Glacier2::ClientRouterI::~ClientRouterI()
{
}

void
Glacier2::ClientRouterI::destroy()
{
}

ObjectPrx
Glacier2::ClientRouterI::getClientProxy(const Current&) const
{
    return _clientAdapter->createProxy(stringToIdentity("dummy"));
}

ObjectPrx
Glacier2::ClientRouterI::getServerProxy(const Current&) const
{
    if(_serverAdapter)
    {
	return _serverAdapter->createProxy(stringToIdentity("dummy"));
    }
    else
    {
	return 0;
    }
}

void
Glacier2::ClientRouterI::addProxy(const ObjectPrx& proxy, const Current&)
{
    if(_routingTableTraceLevel)
    {
	Trace out(_logger, "Glacier2");
	out << "adding proxy to routing table:\n" << _clientAdapter->getCommunicator()->proxyToString(proxy);
    }

    _routingTable->add(proxy);
}

void
Glacier2::ClientRouterI::createSession(const std::string&, const std::string&, const Current&)
{
    assert(false); // Must not be called in this router implementation.
}

ClientBlobjectPtr
Glacier2::ClientRouterI::getClientBlobject() const
{
    return _clientBlobject;
}

ServerBlobjectPtr
Glacier2::ClientRouterI::getServerBlobject() const
{
    return _serverBlobject;
}

Glacier2::SessionRouterI::SessionRouterI(const ObjectAdapterPtr& clientAdapter) :
    _logger(clientAdapter->getCommunicator()->getLogger()),
    _clientAdapter(clientAdapter),
    _traceLevel(clientAdapter->getCommunicator()->getProperties()->getPropertyAsInt("Glacier2.Trace.Session")),
    _serverAdapterCount(0),
    _clientRouterMapHint(_clientRouterMap.end())    
{
}

Glacier2::SessionRouterI::~SessionRouterI()
{
}

void
Glacier2::SessionRouterI::destroy()
{
}

ObjectPrx
Glacier2::SessionRouterI::getClientProxy(const Current& current) const
{
    return getClientRouter(current.transport)->getClientProxy(current); // Forward to the per-client router.
}

ObjectPrx
Glacier2::SessionRouterI::getServerProxy(const Current& current) const
{
    return getClientRouter(current.transport)->getServerProxy(current); // Forward to the per-client router.
}

void
Glacier2::SessionRouterI::addProxy(const ObjectPrx& proxy, const Current& current)
{
    getClientRouter(current.transport)->addProxy(proxy, current); // Forward to the per-client router.
}

void
Glacier2::SessionRouterI::createSession(const std::string&, const std::string&, const Current& current)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);

    //
    // Create a server object adapter only if server endpoints are
    // defined.
    //
    CommunicatorPtr communicator = _clientAdapter->getCommunicator();
    PropertiesPtr properties = communicator->getProperties();
    string endpoints = properties->getProperty("Glacier2.Server.Endpoints");

    ObjectAdapterPtr serverAdapter;
    if(!endpoints.empty())
    {
	ostringstream name;
	name << "Glacier2.Server." << _serverAdapterCount++;
	serverAdapter = communicator->createObjectAdapterWithEndpoints(name.str(), endpoints);
    }
    
    //
    // Add a new client router.
    //
    ClientRouterIPtr clientRouter = new ClientRouterI(_clientAdapter, serverAdapter, current.transport);
    _clientRouterMapHint = _clientRouterMap.insert(_clientRouterMapHint,
						   pair<const TransportInfoPtr, ClientRouterIPtr>(current.transport,
												  clientRouter));
    
    if(_traceLevel >= 1)
    {
	Trace out(_logger, "Glacier2");
	out << "added session for:\n";
	out << current.transport->toString();
	if(serverAdapter)
	{
	    Identity ident;
	    ident.name = "dummy";
	    string endpts = _clientAdapter->getCommunicator()->proxyToString(serverAdapter->createProxy(ident));
	    endpts.erase(0, endpts.find(':') + 1);
	    out << "\nserver adapter endpoints: " << endpts;
	}
    }
}

ClientRouterIPtr
Glacier2::SessionRouterI::getClientRouter(const TransportInfoPtr& transport) const
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);

    if(_clientRouterMapHint != _clientRouterMap.end() && _clientRouterMapHint->first == transport)
    {
	return _clientRouterMapHint->second;
    }
    
    map<TransportInfoPtr, ClientRouterIPtr>::iterator p =
	const_cast<map<TransportInfoPtr, ClientRouterIPtr>&>(_clientRouterMap).find(transport);

    if(p != _clientRouterMap.end())
    {
	_clientRouterMapHint = p;
	return p->second;
    }
    else
    {
	return 0;
    }
}
