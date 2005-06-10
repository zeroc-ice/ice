// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEGRID_OBSERVER_SESSIONI_H
#define ICEGRID_OBSERVER_SESSIONI_H

#include <IceUtil/Mutex.h>

#include <IceGrid/Observer.h>
#include <IceGrid/SessionI.h>
#include <IceGrid/Topics.h>

namespace IceGrid
{

class ObserverSessionI : public Session, public SessionI, public IceUtil::Mutex
{
public:

    ObserverSessionI(const std::string&, RegistryObserverTopic&, NodeObserverTopic&);

    virtual void setObservers(const RegistryObserverPrx&, const NodeObserverPrx&, const Ice::Current&);
    virtual void destroy(const Ice::Current&);

protected:

    const std::string _userId;
    bool _destroyed;

private:
    
    RegistryObserverTopic& _registryObserverTopic;
    NodeObserverTopic& _nodeObserverTopic;

    RegistryObserverPrx _registryObserver;
    NodeObserverPrx _nodeObserver;
};

class LocalObserverSessionI : public ObserverSessionI
{
public:

    LocalObserverSessionI(const std::string&, RegistryObserverTopic&, NodeObserverTopic&);

    virtual void keepAlive(const Ice::Current&);

    virtual IceUtil::Time timestamp() const;

private:

    IceUtil::Time _timestamp;
};

class Glacier2ObserverSessionI : public ObserverSessionI
{
public:

    Glacier2ObserverSessionI(const std::string&, RegistryObserverTopic&, NodeObserverTopic&);

    virtual void keepAlive(const Ice::Current&);

    virtual IceUtil::Time timestamp() const;
};

};

#endif
