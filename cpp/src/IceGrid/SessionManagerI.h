// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEGRID_SESSIONMANAGERI_H
#define ICEGRID_SESSIONMANAGERI_H

#include <IceStorm/IceStorm.h>
#include <Glacier2/Session.h>
#include <IceGrid/Observer.h>
#include <IceGrid/Topics.h>

namespace IceGrid
{

class RegistryI;
typedef IceUtil::Handle<RegistryI> RegistryIPtr;

class ReapThread;
typedef IceUtil::Handle<ReapThread> ReapThreadPtr;

class Database;
typedef IceUtil::Handle<Database> DatabasePtr;

class SessionManagerI : virtual public SessionManager
{
public:

    SessionManagerI(RegistryObserverTopic&, NodeObserverTopic&, const  DatabasePtr&, const ReapThreadPtr&, int);
    
    virtual Glacier2::SessionPrx create(const std::string&, const Ice::Current&);
    virtual SessionPrx createLocalSession(const std::string&, const Ice::Current&);

private:

    RegistryObserverTopic& _registryObserverTopic;
    NodeObserverTopic& _nodeObserverTopic;
    const DatabasePtr _database;
    const ReapThreadPtr _reaper;
    int _sessionTimeout;
};

}

#endif
