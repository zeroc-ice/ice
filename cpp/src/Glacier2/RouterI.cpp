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

using namespace std;
using namespace Ice;
using namespace Glacier2;

class RouterLocator : public ServantLocator
{
public:

    RouterLocator(const ServerBlobjectPtr& serverBlobject) :
	_serverBlobject(serverBlobject)
    {
    }

    virtual ObjectPtr
    locate(const Current& current, LocalObjectPtr&)
    {
	return _serverBlobject;
    }
    
    virtual void
    finished(const Current&, const ObjectPtr&, const LocalObjectPtr&)
    {
    }
    
    virtual void
    deactivate(const string&)
    {
    }

private:

    ServerBlobjectPtr _serverBlobject;
};

Glacier2::RouterI::RouterI(const ObjectAdapterPtr& clientAdapter,
				       const ObjectAdapterPtr& serverAdapter,
				       const TransportInfoPtr& transport) :
    _communicator(clientAdapter->getCommunicator()),
    _routingTable(new IceInternal::RoutingTable),
    _routingTableTraceLevel(_communicator->getProperties()->getPropertyAsInt("Glacier2.Trace.RoutingTable")),
    _clientProxy(clientAdapter->createProxy(stringToIdentity("dummy"))),
    _serverProxy(serverAdapter ? serverAdapter->createProxy(stringToIdentity("dummy")) : ObjectPrx()),
    _clientBlobject(new ClientBlobject(_communicator, _routingTable, "")),
    _serverBlobject(serverAdapter ? new ServerBlobject(_communicator, transport) : 0)
{
    if(serverAdapter)
    {
	assert(_serverBlobject);
	serverAdapter->addServantLocator(new RouterLocator(_serverBlobject), "");
	serverAdapter->activate();
    }
}

Glacier2::RouterI::~RouterI()
{
}

void
Glacier2::RouterI::destroy()
{
    _clientBlobject->destroy();

    if(_serverBlobject)
    {
	_serverBlobject->destroy();
    }	
}

ObjectPrx
Glacier2::RouterI::getClientProxy(const Current&) const
{
    return _clientProxy;
}

ObjectPrx
Glacier2::RouterI::getServerProxy(const Current&) const
{
    return _serverProxy;
}

void
Glacier2::RouterI::addProxy(const ObjectPrx& proxy, const Current&)
{

    if(_routingTableTraceLevel)
    {
	Trace out(_communicator->getLogger(), "Glacier2");
	out << "adding proxy to routing table:\n" << _communicator->proxyToString(proxy);
    }

    _routingTable->add(proxy);
}

void
Glacier2::RouterI::createSession(const std::string&, const std::string&, const Current&)
{
    assert(false); // Must not be called in this router implementation.
}

ClientBlobjectPtr
Glacier2::RouterI::getClientBlobject() const
{
    return _clientBlobject;
}

ServerBlobjectPtr
Glacier2::RouterI::getServerBlobject() const
{
    return _serverBlobject;
}
