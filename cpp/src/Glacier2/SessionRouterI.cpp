// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Glacier2/SessionRouterI.h>
#include <Glacier2/RouterI.h>

using namespace std;
using namespace Ice;
using namespace Glacier2;

class SessionLocator : public ServantLocator
{
public:

    SessionLocator(const SessionRouterIPtr& sessionRouter) :
	_sessionRouter(sessionRouter)
    {
    }
    
    virtual ObjectPtr
    locate(const Current& current, LocalObjectPtr&)
    {
	assert(current.transport);
	return _sessionRouter->getRouter(current.transport)->getClientBlobject();
    }

    virtual void
    finished(const Current&, const ObjectPtr&, const LocalObjectPtr&)
    {
    }

    virtual void
    deactivate(const std::string&)
    {
    }

private:

    const SessionRouterIPtr _sessionRouter;
};

Glacier2::SessionRouterI::SessionRouterI(const ObjectAdapterPtr& clientAdapter) :
    _logger(clientAdapter->getCommunicator()->getLogger()),
    _clientAdapter(clientAdapter),
    _traceLevel(clientAdapter->getCommunicator()->getProperties()->getPropertyAsInt("Glacier2.Trace.Session")),
    _serverAdapterCount(0),
    _routersHint(_routers.end()),
    _destroy(false)
{
    _clientAdapter->addServantLocator(new SessionLocator(this), "");
}

Glacier2::SessionRouterI::~SessionRouterI()
{
    assert(_destroy);
    assert(_routers.empty());
}

void
Glacier2::SessionRouterI::destroy()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);

    assert(!_destroy);

    _destroy = true;

    for_each(_routers.begin(), _routers.end(),
	     Ice::secondVoidMemFun<const TransportInfoPtr, RouterI>(&RouterI::destroy));
    _routers.clear();
    _routersHint = _routers.end();

    notify();
}

ObjectPrx
Glacier2::SessionRouterI::getClientProxy(const Current& current) const
{
    assert(!_destroy);
    return getRouter(current.transport)->getClientProxy(current); // Forward to the per-client router.
}

ObjectPrx
Glacier2::SessionRouterI::getServerProxy(const Current& current) const
{
    assert(!_destroy);
    return getRouter(current.transport)->getServerProxy(current); // Forward to the per-client router.
}

void
Glacier2::SessionRouterI::addProxy(const ObjectPrx& proxy, const Current& current)
{
    assert(!_destroy);
    getRouter(current.transport)->addProxy(proxy, current); // Forward to the per-client router.
}

void
Glacier2::SessionRouterI::createSession(const std::string&, const std::string&, const Current& current)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);

    assert(!_destroy);

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
    RouterIPtr router = new RouterI(_clientAdapter, serverAdapter, current.transport);
    _routersHint = _routers.insert(_routersHint, pair<const TransportInfoPtr, RouterIPtr>(current.transport, router));
    
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

RouterIPtr
Glacier2::SessionRouterI::getRouter(const TransportInfoPtr& transport) const
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);

    assert(!_destroy);

    if(_routersHint != _routers.end() && _routersHint->first == transport)
    {
	return _routersHint->second;
    }
    
    map<TransportInfoPtr, RouterIPtr>::iterator p =
	const_cast<map<TransportInfoPtr, RouterIPtr>&>(_routers).find(transport);

    if(p != _routers.end())
    {
	_routersHint = p;
	return p->second;
    }
    else
    {
	return 0;
    }
}
