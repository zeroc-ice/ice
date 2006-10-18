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
#include <IceGrid/ReapThread.h>

namespace IceGrid
{

class RegistryI;
typedef IceUtil::Handle<RegistryI> RegistryIPtr;

class AdminSessionI : public BaseSessionI, public AdminSession
{
public:

    AdminSessionI(const std::string&, const DatabasePtr&, int, const std::string&);
    virtual ~AdminSessionI();

    void setAdmin(const AdminPrx&);

    virtual void keepAlive(const Ice::Current& current) { BaseSessionI::keepAlive(current); }

    virtual AdminPrx getAdmin(const Ice::Current&) const;

    virtual void setObservers(const RegistryObserverPrx&, const NodeObserverPrx&, const ApplicationObserverPrx&,
			      const AdapterObserverPrx&, const ObjectObserverPrx&, const Ice::Current&);

    virtual void setObserversByIdentity(const Ice::Identity&, const Ice::Identity&, const Ice::Identity&,
					const Ice::Identity&, const Ice::Identity&, const Ice::Current&); 

    virtual int startUpdate(const Ice::Current&);
    virtual void finishUpdate(const Ice::Current&);

    virtual std::string getReplicaName(const Ice::Current&) const;

    virtual void destroy(const Ice::Current&);

private:

    void setupObserverSubscription(TopicName, const Ice::ObjectPrx&);
    Ice::ObjectPrx toProxy(const Ice::Identity&, const Ice::ConnectionPtr&);

    const int _timeout;
    const AdminPrx _admin;
    const std::string _replicaName;
    std::map<TopicName, Ice::ObjectPrx> _observers;
};
typedef IceUtil::Handle<AdminSessionI> AdminSessionIPtr;

class AdminSessionFactory : virtual public IceUtil::Shared
{
public:

    AdminSessionFactory(const Ice::ObjectAdapterPtr&, const DatabasePtr&, const ReapThreadPtr&, const RegistryIPtr&);
    
    Glacier2::SessionPrx createGlacier2Session(const std::string&, const Glacier2::SessionControlPrx&);
    AdminSessionIPtr createSessionServant(const std::string&);

    const TraceLevelsPtr& getTraceLevels() const;

private:

    const Ice::ObjectAdapterPtr _adapter;
    const DatabasePtr _database;
    const int _timeout;
    const WaitQueuePtr _waitQueue;
    const ReapThreadPtr _reaper;
    const RegistryIPtr _registry;
};
typedef IceUtil::Handle<AdminSessionFactory> AdminSessionFactoryPtr;

class AdminSessionManagerI : virtual public Glacier2::SessionManager
{
public:

    AdminSessionManagerI(const AdminSessionFactoryPtr&);
    
    virtual Glacier2::SessionPrx create(const std::string&, const Glacier2::SessionControlPrx&, const Ice::Current&);

private:
    
    const AdminSessionFactoryPtr _factory;
};

class AdminSSLSessionManagerI : virtual public Glacier2::SSLSessionManager
{
public:

    AdminSSLSessionManagerI(const AdminSessionFactoryPtr&);
    virtual Glacier2::SessionPrx create(const Glacier2::SSLInfo&, const Glacier2::SessionControlPrx&, 
					const Ice::Current&);

private:
    
    const AdminSessionFactoryPtr _factory;
};

};

#endif
