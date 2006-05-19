// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEGRID_SESSIONI_H
#define ICEGRID_SESSIONI_H

#include <IceUtil/Mutex.h>
#include <IceGrid/Session.h>

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

class WaitQueue;
typedef IceUtil::Handle<WaitQueue> WaitQueuePtr;

class BaseSessionI;
typedef IceUtil::Handle<BaseSessionI> BaseSessionIPtr;

class SessionI;
typedef IceUtil::Handle<SessionI> SessionIPtr;

class BaseSessionI : virtual public BaseSession, public IceUtil::Mutex
{
public:

    virtual ~BaseSessionI();

    virtual void keepAlive(const Ice::Current&);
    virtual int getTimeout(const Ice::Current&) const;
    virtual void destroy(const Ice::Current&);

    IceUtil::Time timestamp() const;

protected:

    BaseSessionI(const std::string&, const std::string&, const DatabasePtr&, int);

    const std::string _id;
    const std::string _prefix;
    const int _timeout;
    const TraceLevelsPtr _traceLevels;
    const DatabasePtr _database;
    bool _destroyed;
    IceUtil::Time _timestamp;
};

class SessionI : virtual public Session, public BaseSessionI
{
public:

    SessionI(const std::string&, const DatabasePtr&, int, const WaitQueuePtr&, const Glacier2::SessionControlPrx&);
    SessionI(const std::string&, const DatabasePtr&, int, const WaitQueuePtr&, const Ice::ConnectionPtr&);
    virtual ~SessionI();

    virtual void allocateObjectById_async(const AMD_Session_allocateObjectByIdPtr&, const Ice::Identity&,
					  const Ice::Current&);
    virtual void allocateObjectByType_async(const AMD_Session_allocateObjectByTypePtr&, const std::string&,
					    const Ice::Current&);
    virtual void releaseObject(const Ice::Identity&, const Ice::Current&);
    virtual void setAllocationTimeout(int, const Ice::Current&);
    virtual void destroy(const Ice::Current&);

    int getAllocationTimeout() const;
    const WaitQueuePtr& getWaitQueue() const { return _waitQueue; }
    const std::string& getId() const { return _id; }

    bool addAllocationRequest(const AllocationRequestPtr&);
    void removeAllocationRequest(const AllocationRequestPtr&);
    bool addAllocation(const AllocatablePtr&);
    void removeAllocation(const AllocatablePtr&);

protected:

    const WaitQueuePtr _waitQueue;
    const Glacier2::SessionControlPrx _sessionControl;
    const Ice::ConnectionPtr _connection;
    int _allocationTimeout;
    std::set<AllocationRequestPtr> _requests;
    std::set<AllocatablePtr> _allocations;
};

class ClientSessionManagerI : virtual public Glacier2::SessionManager
{
public:

    ClientSessionManagerI(const  DatabasePtr&, int, const WaitQueuePtr&);
    
    virtual Glacier2::SessionPrx create(const std::string&, const Glacier2::SessionControlPrx&, const Ice::Current&);

    SessionIPtr create(const std::string&, const Glacier2::SessionControlPrx&);

private:

    const DatabasePtr _database;
    const int _timeout;
    const WaitQueuePtr _waitQueue;
};
typedef IceUtil::Handle<ClientSessionManagerI> ClientSessionManagerIPtr;

class ClientSSLSessionManagerI : virtual public Glacier2::SSLSessionManager
{
public:

    ClientSSLSessionManagerI(const  DatabasePtr&, int, const WaitQueuePtr&);
    
    virtual Glacier2::SessionPrx create(const Glacier2::SSLInfo&, const Glacier2::SessionControlPrx&, 
					const Ice::Current&);

private:

    const DatabasePtr _database;
    const int _timeout;
    const WaitQueuePtr _waitQueue;
};
typedef IceUtil::Handle<ClientSSLSessionManagerI> ClientSSLSessionManagerIPtr;

};

#endif
