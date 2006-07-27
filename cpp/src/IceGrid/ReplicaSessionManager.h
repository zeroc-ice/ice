// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_REPLICA_SESSION_MANAGER_H
#define ICE_GRID_REPLICA_SESSION_MANAGER_H

#include <IceUtil/Handle.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/Monitor.h>

#include <IceGrid/SessionManager.h>
#include <IceGrid/Internal.h>

namespace IceGrid
{

class Database;
typedef IceUtil::Handle<Database> DatabasePtr;

class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

class ReplicaSessionManager : public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    ReplicaSessionManager();
    
    void create(const std::string&, const DatabasePtr&, const InternalRegistryPrx&);
    void create(const InternalRegistryPrx&);
    void activate();
    NodePrxSeq getNodes() const;
    void destroy();

private:

    class Thread : public SessionKeepAliveThread<ReplicaSessionPrx, InternalRegistryPrx>
    {
    public:

	Thread(ReplicaSessionManager& manager, const InternalRegistryPrx& master) : 
	    SessionKeepAliveThread<ReplicaSessionPrx, InternalRegistryPrx>(master),
	    _manager(manager)
        {
	}

	virtual ReplicaSessionPrx 
	createSession(const InternalRegistryPrx& master, IceUtil::Time& timeout) const
        {
	    return _manager.createSession(master, timeout);
	}

	virtual void 
	destroySession(const ReplicaSessionPrx& session) const
        {
	    _manager.destroySession(session);
	}

	virtual bool 
	keepAlive(const ReplicaSessionPrx& session) const
        {
	    return _manager.keepAlive(session);
	}

    private:
	
	ReplicaSessionManager& _manager;
    };
    typedef IceUtil::Handle<Thread> ThreadPtr;

    friend class Thread;

    ReplicaSessionPrx createSession(const InternalRegistryPrx&, IceUtil::Time&) const;
    void destroySession(const ReplicaSessionPrx&) const;
    bool keepAlive(const ReplicaSessionPrx&) const;

    ThreadPtr _thread;
    std::string _name;
    InternalRegistryPrx _master;
    InternalRegistryPrx _replica;
    RegistryObserverPrx _observer;
    DatabasePtr _database;
    TraceLevelsPtr _traceLevels;
};

}

#endif
