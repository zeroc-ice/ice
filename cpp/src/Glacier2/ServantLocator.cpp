// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Glacier2/ServantLocator.h>

using namespace std;
using namespace Ice;
using namespace Glacier2;

Glacier2::ClientServantLocator::ClientServantLocator(const ObjectAdapterPtr& clientAdapter) :
    _routerId(stringToIdentity(clientAdapter->getCommunicator()->getProperties()->
			       getPropertyWithDefault("Glacier2.Identity", "Glacier2/router"))),
    _sessionRouter(new SessionRouterI(clientAdapter))
{
}

ObjectPtr
Glacier2::ClientServantLocator::locate(const Current& current, LocalObjectPtr&)
{
    assert(current.transport);

    if(current.id == _routerId)
    {
	return _sessionRouter;
    }
    else
    {
	return _sessionRouter->getClientRouter(current.transport)->getClientBlobject();
    }
}

void
Glacier2::ClientServantLocator::finished(const Current&, const ObjectPtr&, const LocalObjectPtr&)
{
}

void
Glacier2::ClientServantLocator::deactivate(const string&)
{
}

Glacier2::ServerServantLocator::ServerServantLocator(const ServerBlobjectPtr& serverBlobject) :
    _serverBlobject(serverBlobject)
{
}

ObjectPtr
Glacier2::ServerServantLocator::locate(const Current& current, LocalObjectPtr&)
{
    return _serverBlobject;
}

void
Glacier2::ServerServantLocator::finished(const Current&, const ObjectPtr&, const LocalObjectPtr&)
{
}

void
Glacier2::ServerServantLocator::deactivate(const string&)
{
}
