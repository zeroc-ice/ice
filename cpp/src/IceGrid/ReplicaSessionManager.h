// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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

class WellKnownObjectsManager;
typedef IceUtil::Handle<WellKnownObjectsManager> WellKnownObjectsManagerPtr;

class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

class ReplicaSessionManager : public SessionManager
{
public:

    class Thread : public SessionKeepAliveThread<ReplicaSessionPrx>
    {
    public:

        Thread(ReplicaSessionManager& manager, const InternalRegistryPrx& master, const Ice::LoggerPtr& logger) :
            SessionKeepAliveThread<ReplicaSessionPrx>(master, logger),
            _manager(manager)
        {
        }

        virtual ReplicaSessionPrx
        createSession(InternalRegistryPrx& master, IceUtil::Time& timeout)
        {
            return _manager.createSession(master, timeout);
        }

        virtual void
        destroySession(const ReplicaSessionPrx& session)
        {
            _manager.destroySession(session);
        }

        virtual bool
        keepAlive(const ReplicaSessionPrx& session)
        {
            return _manager.keepAlive(session);
        }

        void registerAllWellKnownObjects();

    private:

        ReplicaSessionManager& _manager;
    };
    typedef IceUtil::Handle<Thread> ThreadPtr;

    ReplicaSessionManager(const Ice::CommunicatorPtr&, const std::string&);
    void create(const std::string&, const InternalReplicaInfoPtr&, const DatabasePtr&,
                const WellKnownObjectsManagerPtr&, const InternalRegistryPrx&);
    void create(const InternalRegistryPrx&);
    NodePrxSeq getNodes(const NodePrxSeq&) const;
    void destroy();

    void registerAllWellKnownObjects();
    ReplicaSessionPrx getSession() const { return _thread ? _thread->getSession() : ReplicaSessionPrx(); }

    IceGrid::InternalRegistryPrx findInternalRegistryForReplica(const Ice::Identity&);

private:

    friend class Thread;

    bool isDestroyed()
    {
        Lock sync(*this);
        return !_communicator;
    }

    ReplicaSessionPrx createSession(InternalRegistryPrx&, IceUtil::Time&);
    ReplicaSessionPrx createSessionImpl(const InternalRegistryPrx&, IceUtil::Time&);
    void destroySession(const ReplicaSessionPrx&);
    bool keepAlive(const ReplicaSessionPrx&);

    ThreadPtr _thread;
    std::string _name;
    InternalReplicaInfoPtr _info;
    RegistryPrx _registry;
    InternalRegistryPrx _internalRegistry;
    DatabaseObserverPrx _observer;
    DatabasePtr _database;
    WellKnownObjectsManagerPtr _wellKnownObjects;
    TraceLevelsPtr _traceLevels;
};

}

#endif
