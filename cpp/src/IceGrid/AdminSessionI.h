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

class RegistryI;
typedef IceUtil::Handle<RegistryI> RegistryIPtr;

class AdminSessionI : public BaseSessionI, public AdminSession
{
public:

    AdminSessionI(const std::string&, const DatabasePtr&, int, const RegistryObserverTopicPtr&, 
		  const NodeObserverTopicPtr&);
    virtual ~AdminSessionI();

    void setAdmin(const AdminPrx&);

    virtual void keepAlive(const Ice::Current& current) { BaseSessionI::keepAlive(current); }
    virtual int getTimeout(const Ice::Current& current) const { return BaseSessionI::getTimeout(current); }

    virtual AdminPrx getAdmin(const Ice::Current&) const;

    virtual void setObservers(const RegistryObserverPrx&, const NodeObserverPrx&, const Ice::Current&);
    virtual void setObserversByIdentity(const Ice::Identity&, const Ice::Identity&, const Ice::Current&); 

    virtual int startUpdate(const Ice::Current&);
    virtual void finishUpdate(const Ice::Current&);

    virtual void destroy(const Ice::Current&);

private:
    
    const RegistryObserverTopicPtr _registryObserverTopic;
    const NodeObserverTopicPtr _nodeObserverTopic;
    const AdminPrx _admin;

    RegistryObserverPrx _registryObserver;
    NodeObserverPrx _nodeObserver;
};
typedef IceUtil::Handle<AdminSessionI> AdminSessionIPtr;

class AdminSessionManagerI : virtual public Glacier2::SessionManager
{
public:

    AdminSessionManagerI(const  DatabasePtr&, int, const RegistryObserverTopicPtr& , const NodeObserverTopicPtr&, 
			 const RegistryIPtr&);
    
    virtual Glacier2::SessionPrx create(const std::string&, const Glacier2::SessionControlPrx&, const Ice::Current&);
    AdminSessionIPtr create(const std::string&);

private:
    
    const DatabasePtr _database;
    const int _timeout;
    const RegistryObserverTopicPtr _registryObserverTopic;
    const NodeObserverTopicPtr _nodeObserverTopic;
    const RegistryIPtr _registry;
};
typedef IceUtil::Handle<AdminSessionManagerI> AdminSessionManagerIPtr;

class AdminSSLSessionManagerI : virtual public Glacier2::SSLSessionManager
{
public:

    AdminSSLSessionManagerI(const  DatabasePtr&, int, const RegistryObserverTopicPtr& , const NodeObserverTopicPtr&,
			    const RegistryIPtr&);
    virtual Glacier2::SessionPrx create(const Glacier2::SSLInfo&, const Glacier2::SessionControlPrx&, 
					const Ice::Current&);

private:
    
    const DatabasePtr _database;
    const int _timeout;
    const RegistryObserverTopicPtr _registryObserverTopic;
    const NodeObserverTopicPtr _nodeObserverTopic;
    const RegistryIPtr _registry;
};
typedef IceUtil::Handle<AdminSSLSessionManagerI> AdminSSLSessionManagerIPtr;

};

#endif
