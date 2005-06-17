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

class Database;
typedef IceUtil::Handle<Database> DatabasePtr;

class ObserverSessionI : public Session, public SessionI, public IceUtil::Mutex
{
public:

    ObserverSessionI(const std::string&, const DatabasePtr&, RegistryObserverTopic&, NodeObserverTopic&);

    virtual void setObservers(const RegistryObserverPrx&, const NodeObserverPrx&, const Ice::Current&);
    virtual void setObserversByIdentity(const Ice::Identity&, const Ice::Identity&, const Ice::Current&); 

    virtual void startUpdate(int, const Ice::Current&);
    virtual void addApplication(const ApplicationDescriptorPtr&, const Ice::Current&);
    virtual void syncApplication(const ApplicationDescriptorPtr&, const Ice::Current&);
    virtual void updateApplication(const ApplicationUpdateDescriptor&, const Ice::Current&);
    virtual void removeApplication(const std::string&, const Ice::Current&);
    virtual void finishUpdate(const Ice::Current&);

    virtual void destroy(const Ice::Current&);

protected:

    const std::string _userId;
    bool _updating;
    bool _destroyed;

private:
    
    const DatabasePtr _database;
    RegistryObserverTopic& _registryObserverTopic;
    NodeObserverTopic& _nodeObserverTopic;
    
    RegistryObserverPrx _registryObserver;
    NodeObserverPrx _nodeObserver;
};

class LocalObserverSessionI : public ObserverSessionI
{
public:

    LocalObserverSessionI(const std::string&, const DatabasePtr&, RegistryObserverTopic&, NodeObserverTopic&);

    virtual void keepAlive(const Ice::Current&);

    virtual IceUtil::Time timestamp() const;

private:

    IceUtil::Time _timestamp;
};

class Glacier2ObserverSessionI : public ObserverSessionI
{
public:

    Glacier2ObserverSessionI(const std::string&, const DatabasePtr&, RegistryObserverTopic&, NodeObserverTopic&);

    virtual void keepAlive(const Ice::Current&);

    virtual IceUtil::Time timestamp() const;
};

};

#endif
