// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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

using namespace std;
using namespace IceGrid;

SessionManagerI::SessionManagerI(const IceStorm::TopicPrx& registryObserverTopic, 
				 const IceStorm::TopicPrx& nodeObserverTopic,
				 const ReapThreadPtr& reaper) :
    _registryObserverTopic(registryObserverTopic),
    _nodeObserverTopic(nodeObserverTopic),
    _reaper(reaper)
{
}

Glacier2::SessionPrx
SessionManagerI::create(const string& userId, const Ice::Current& current)
{
    SessionIPtr session = new Glacier2ObserverSessionI(userId, _registryObserverTopic, _nodeObserverTopic);
    Glacier2::SessionPrx proxy = Glacier2::SessionPrx::uncheckedCast(current.adapter->addWithUUID(session));
    _reaper->add(proxy, session);
    return proxy;
}

SessionPrx
SessionManagerI::createLocalSession(const string& userId, const Ice::Current& current)
{
    SessionIPtr session = new LocalObserverSessionI(userId, _registryObserverTopic, _nodeObserverTopic);
    SessionPrx proxy = SessionPrx::uncheckedCast(current.adapter->addWithUUID(session));
    _reaper->add(proxy, session);
    return proxy;
}

