// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/RoutingTable.h>
#include <Glacier2/ClientServantLocator.h>
#include <Glacier2/RouterI.h>

using namespace std;
using namespace Ice;
using namespace Glacier;

Glacier::ClientServantLocator::ClientServantLocator(const ObjectAdapterPtr& clientAdapter,
						    const ObjectAdapterPtr& serverAdapter,
						    const SessionManagerPrx& sessionManager) :
    _routingTable(new IceInternal::RoutingTable),
    _routerId(stringToIdentity(clientAdapter->getCommunicator()->getProperties()->getPropertyWithDefault(
	"Glacier.Router.Identity", "Glacier/router"))),
    _router(new RouterI(clientAdapter, serverAdapter, _routingTable, sessionManager, "todo")),
    _blobject(new ClientBlobject(clientAdapter->getCommunicator(), _routingTable, ""))
{
}

ObjectPtr
Glacier::ClientServantLocator::locate(const Current& current, LocalObjectPtr&)
{
    if(current.id == _routerId)
    {
	return _router;
    }
    else
    {
	return _blobject;
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
    assert(_blobject);
    _blobject->destroy();
    _blobject = 0;

    assert(_router);
    RouterI* router = dynamic_cast<RouterI*>(_router.get());
    assert(router);
    router->destroy();
    _router = 0;
}
