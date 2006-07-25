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
#include <IceUtil/Thread.h>

#include <IceGrid/Internal.h>

namespace IceGrid
{

class Database;
typedef IceUtil::Handle<Database> DatabasePtr;

class ReplicaSessionKeepAliveThread : public IceUtil::Thread, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    ReplicaSessionKeepAliveThread(const std::string&, const InternalRegistryPrx&, const InternalRegistryPrx&, 
				  const ReplicaInfo&, const DatabasePtr&);

    virtual void run();

    void terminate();

private:

    void keepAlive(const ReplicaSessionPrx&);

    const std::string _name;
    const InternalRegistryPrx _master;
    const InternalRegistryPrx _replica;
    const ReplicaInfo _info;
    const DatabasePtr _database;
    bool _shutdown;
};
typedef IceUtil::Handle<ReplicaSessionKeepAliveThread> ReplicaSessionKeepAliveThreadPtr;

class ReplicaSessionManager
{
public:

    ReplicaSessionManager();
    
    void create(const std::string&, 
		const DatabasePtr&, 
		const InternalRegistryPrx&, 
		const Ice::ObjectAdapterPtr&, 
		const Ice::ObjectAdapterPtr&);

    void destroy();

private:

    ReplicaSessionKeepAliveThreadPtr _session;
};

}

#endif
