// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEGRID_SESSIONI_H
#define ICEGRID_SESSIONI_H

#include <IceUtil/Mutex.h>
#include <IceUtil/Timer.h>
#include <IceGrid/ReapThread.h>
#include <IceGrid/Session.h>
#include <IceGrid/SessionServantManager.h>
#include <set>

namespace IceGrid
{

class Database;
typedef IceUtil::Handle<Database> DatabasePtr;

class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

class AllocationRequest;
typedef IceUtil::Handle<AllocationRequest> AllocationRequestPtr;

class Allocatable;
typedef IceUtil::Handle<Allocatable> AllocatablePtr;

class SessionI;
typedef IceUtil::Handle<SessionI> SessionIPtr;

class BaseSessionI : public virtual Ice::Object, public IceUtil::Mutex
{
public:

    virtual ~BaseSessionI();

    virtual void keepAlive(const Ice::Current&);

    IceUtil::Time timestamp() const;
    void shutdown();
    Glacier2::IdentitySetPrx getGlacier2IdentitySet();
    Glacier2::StringSetPrx getGlacier2AdapterIdSet();

    const std::string& getId() const { return _id; }
    virtual void destroyImpl(bool);

protected:

    BaseSessionI(const std::string&, const std::string&, const DatabasePtr&);

    const std::string _id;
    const std::string _prefix;
    const TraceLevelsPtr _traceLevels;
    const DatabasePtr _database;
    SessionServantManagerPtr _servantManager;
    bool _destroyed;
    IceUtil::Time _timestamp;
};
typedef IceUtil::Handle<BaseSessionI> BaseSessionIPtr;

class SessionDestroyedException
{
};

class SessionI : public BaseSessionI, public Session
{
public:

    SessionI(const std::string&, const DatabasePtr&, const IceUtil::TimerPtr&);
    virtual ~SessionI();

    Ice::ObjectPrx _register(const SessionServantManagerPtr&, const Ice::ConnectionPtr&);

    virtual void keepAlive(const Ice::Current& current) { BaseSessionI::keepAlive(current); }

    virtual void allocateObjectById_async(const AMD_Session_allocateObjectByIdPtr&, const Ice::Identity&,
                                          const Ice::Current&);
    virtual void allocateObjectByType_async(const AMD_Session_allocateObjectByTypePtr&, const std::string&,
                                            const Ice::Current&);
    virtual void releaseObject(const Ice::Identity&, const Ice::Current&);
    virtual void setAllocationTimeout(int, const Ice::Current&);
    virtual void destroy(const Ice::Current&);

    int getAllocationTimeout() const;
    const IceUtil::TimerPtr& getTimer() const { return _timer; }

    bool addAllocationRequest(const AllocationRequestPtr&);
    void removeAllocationRequest(const AllocationRequestPtr&);
    void addAllocation(const AllocatablePtr&);
    void removeAllocation(const AllocatablePtr&);

protected:

    virtual void destroyImpl(bool);

    const IceUtil::TimerPtr _timer;
    int _allocationTimeout;
    std::set<AllocationRequestPtr> _requests;
    std::set<AllocatablePtr> _allocations;
};

class ClientSessionFactory : public virtual IceUtil::Shared
{
public:

    ClientSessionFactory(const SessionServantManagerPtr&, const DatabasePtr&, const IceUtil::TimerPtr&,
                         const ReapThreadPtr&);

    Glacier2::SessionPrx createGlacier2Session(const std::string&, const Glacier2::SessionControlPrx&);
    SessionIPtr createSessionServant(const std::string&, const Glacier2::SessionControlPrx&);

    const TraceLevelsPtr& getTraceLevels() const;

private:

    const SessionServantManagerPtr _servantManager;
    const DatabasePtr _database;
    const IceUtil::TimerPtr _timer;
    const ReapThreadPtr _reaper;
    const bool _filters;
};
typedef IceUtil::Handle<ClientSessionFactory> ClientSessionFactoryPtr;

class ClientSessionManagerI : public virtual Glacier2::SessionManager
{
public:

    ClientSessionManagerI(const ClientSessionFactoryPtr&);

    virtual Glacier2::SessionPrx create(const std::string&, const Glacier2::SessionControlPrx&, const Ice::Current&);

private:

    const ClientSessionFactoryPtr _factory;
};

class ClientSSLSessionManagerI : public virtual Glacier2::SSLSessionManager
{
public:

    ClientSSLSessionManagerI(const  ClientSessionFactoryPtr&);

    virtual Glacier2::SessionPrx create(const Glacier2::SSLInfo&, const Glacier2::SessionControlPrx&,
                                        const Ice::Current&);

private:

    const ClientSessionFactoryPtr _factory;
};

};

#endif
