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

Glacier2::SessionRouterI::SessionRouterI(const ObjectAdapterPtr& clientAdapter,
					 const ObjectAdapterPtr& serverAdapter) :
    _logger(clientAdapter->getCommunicator()->getLogger()),
    _clientAdapter(clientAdapter),
    _serverAdapter(serverAdapter),
    _traceLevel(clientAdapter->getCommunicator()->getProperties()->getPropertyAsInt("Glacier2.Trace.Session")),
    _sessionThread(new SessionThread(this)),
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

    _sessionThread->getThreadControl().join();
}

ObjectPrx
Glacier2::SessionRouterI::getClientProxy(const Current& current) const
{
    return getRouter(current.transport)->getClientProxy(current); // Forward to the per-client router.
}

ObjectPrx
Glacier2::SessionRouterI::getServerProxy(const Current& current) const
{
    return getRouter(current.transport)->getServerProxy(current); // Forward to the per-client router.
}

void
Glacier2::SessionRouterI::addProxy(const ObjectPrx& proxy, const Current& current)
{
    getRouter(current.transport)->addProxy(proxy, current); // Forward to the per-client router.
}

void
Glacier2::SessionRouterI::createSession(const std::string& userId, const std::string& password, const Current& current)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);

    assert(!_destroy);

    //
    // Add a new per-client router.
    //
    RouterIPtr router = new RouterI(_clientAdapter, _serverAdapter, current.transport);
    _routersHint = _routers.insert(_routersHint, pair<const TransportInfoPtr, RouterIPtr>(current.transport, router));
    
    if(_traceLevel >= 1)
    {
	Trace out(_logger, "Glacier2");
	out << "added session for:\n";
	out << current.transport->toString();
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

void
Glacier2::SessionRouterI::run()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);

    while(!_destroy)
    {		
	wait();
    }
}

Glacier2::SessionRouterI::SessionThread::SessionThread(const SessionRouterIPtr& sessionRouter) :
    _sessionRouter(sessionRouter)
{
}

void
Glacier2::SessionRouterI::SessionThread::SessionThread::run()
{
    _sessionRouter->run();
    _sessionRouter = 0; // Break cyclic dependencies.
}
