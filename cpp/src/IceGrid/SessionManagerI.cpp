// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceGrid/SessionManagerI.h>
#include <IceGrid/RegistryI.h>
#include <IceGrid/ObserverSessionI.h>
#include <IceGrid/ReapThread.h>
#include <IceGrid/Database.h>

using namespace std;
using namespace IceGrid;

SessionManagerI::SessionManagerI(RegistryObserverTopic& regTopic,
				 NodeObserverTopic& nodeTopic,
				 const DatabasePtr& database,
				 const ReapThreadPtr& reaper,
				 int sessionTimeout) :
    _registryObserverTopic(regTopic),
    _nodeObserverTopic(nodeTopic), 
    _database(database), 
    _reaper(reaper),
    _sessionTimeout(sessionTimeout)
{
}

Glacier2::SessionPrx
SessionManagerI::create(const string& userId, const Glacier2::SessionControlPrx&, const Ice::Current& current)
{
    SessionIPtr session =
	new Glacier2ObserverSessionI(userId, _database, _registryObserverTopic, _nodeObserverTopic, _sessionTimeout);
    return Glacier2::SessionPrx::uncheckedCast(current.adapter->addWithUUID(session));
}

SessionPrx
SessionManagerI::createLocalSession(const string& userId, const Ice::Current& current)
{
    SessionIPtr session = 
	new LocalObserverSessionI(userId, _database, _registryObserverTopic, _nodeObserverTopic, _sessionTimeout);
    SessionPrx proxy = SessionPrx::uncheckedCast(current.adapter->addWithUUID(session));
    _reaper->add(proxy, session);
    return proxy;
}

