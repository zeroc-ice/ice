// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEGRID_ADMINSESSIONI_H
#define ICEGRID_ADMINSESSIONI_H

#include <IceGrid/SessionI.h>
#include <IceGrid/Topics.h>

namespace IceGrid
{

class AdminSessionI : public SessionI, public AdminSession
{
public:

    AdminSessionI(const std::string&, const DatabasePtr&, const Ice::ObjectAdapterPtr&, const WaitQueuePtr&,
		  RegistryObserverTopic&, NodeObserverTopic&, int);
    virtual ~AdminSessionI();

    virtual AdminPrx getAdmin(const Ice::Current&) const;

    virtual void setObservers(const RegistryObserverPrx&, const NodeObserverPrx&, const Ice::Current&);
    virtual void setObserversByIdentity(const Ice::Identity&, const Ice::Identity&, const Ice::Current&); 

    virtual int startUpdate(const Ice::Current&);
    virtual void addApplication(const ApplicationDescriptor&, const Ice::Current&);
    virtual void syncApplication(const ApplicationDescriptor&, const Ice::Current&);
    virtual void updateApplication(const ApplicationUpdateDescriptor&, const Ice::Current&);
    virtual void removeApplication(const std::string&, const Ice::Current&);
    virtual void finishUpdate(const Ice::Current&);

    virtual void destroy(const Ice::Current&);

protected:

    bool _updating;

private:
    
    RegistryObserverTopic& _registryObserverTopic;
    NodeObserverTopic& _nodeObserverTopic;
    
    RegistryObserverPrx _registryObserver;
    NodeObserverPrx _nodeObserver;
};

class AdminSessionManagerI : virtual public SessionManager
{
public:

    AdminSessionManagerI(RegistryObserverTopic&, NodeObserverTopic&, const  DatabasePtr&, const ReapThreadPtr&, 
			 const WaitQueuePtr&, int);
    
    virtual Glacier2::SessionPrx create(const std::string&, const Glacier2::SessionControlPrx&, const Ice::Current&);
    virtual SessionPrx createLocalSession(const std::string&, const Ice::Current&);

private:

    RegistryObserverTopic& _registryObserverTopic;
    NodeObserverTopic& _nodeObserverTopic;
    const DatabasePtr _database;
    const ReapThreadPtr _reaper;
    const WaitQueuePtr _waitQueue;
    int _timeout;
};

};

#endif
