// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_NODE_I_H
#define ICE_GRID_NODE_I_H

#include <IceGrid/Internal.h>
#include <IceGrid/WaitQueue.h>
#include <IceGrid/PlatformInfo.h>
#include <IceGrid/UserAccountMapper.h>

#include <IcePatch2/FileServer.h>

namespace IceGrid
{

class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

class Activator;
typedef IceUtil::Handle<Activator> ActivatorPtr;

class ServerI;
typedef IceUtil::Handle<ServerI> ServerIPtr;

class NodeSessionManager;

class NodeI : public Node, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    NodeI(const Ice::ObjectAdapterPtr&, NodeSessionManager&, const ActivatorPtr&, const WaitQueuePtr&, 
	  const TraceLevelsPtr&, const NodePrx&, const std::string&, const UserAccountMapperPrx&);
    virtual ~NodeI();

    virtual void loadServer_async(const AMD_Node_loadServerPtr&, const std::string&, const ServerDescriptorPtr&, 
				  const std::string&, const Ice::Current&);
    virtual void destroyServer_async(const AMD_Node_destroyServerPtr&, const std::string&, const Ice::Current&);
    virtual void patch(const std::string&, const std::string&, const DistributionDescriptor&, bool,
		       const Ice::Current&);

    virtual void replicaAdded(const InternalRegistryPrx&, const Ice::Current&);
    virtual void replicaRemoved(const InternalRegistryPrx&, const Ice::Current&);

    virtual std::string getName(const Ice::Current& = Ice::Current()) const;
    virtual std::string getHostname(const Ice::Current& = Ice::Current()) const;
    virtual LoadInfo getLoad(const Ice::Current& = Ice::Current()) const;
    virtual void shutdown(const Ice::Current&) const;
    
    WaitQueuePtr getWaitQueue() const;
    Ice::CommunicatorPtr getCommunicator() const;
    Ice::ObjectAdapterPtr getAdapter() const;
    ActivatorPtr getActivator() const;
    TraceLevelsPtr getTraceLevels() const;
    NodeObserverPrx getObserver() const;
    UserAccountMapperPrx getUserAccountMapper() const;
    PlatformInfo& getPlatformInfo() const { return _platform; }

    NodeSessionPrx registerWithRegistry(const InternalRegistryPrx&);
    void setObserver(const NodeObserverPrx&);
    void checkConsistency(const NodeSessionPrx&);

private:

    void checkConsistencyNoSync(const Ice::StringSeq&);
    bool canRemoveServerDirectory(const std::string&);
    void initObserver(const Ice::StringSeq&);
    void patch(const IcePatch2::FileServerPrx&, const std::string&, const std::vector<std::string>&);
    
    void addServer(const std::string&, const ServerIPtr&);
    void removeServer(const ServerIPtr&);
    std::set<ServerIPtr> getApplicationServers(const std::string&);
    Ice::Identity createServerIdentity(const std::string&);

    const Ice::ObjectAdapterPtr _adapter;
    NodeSessionManager& _sessions;
    const ActivatorPtr _activator;
    const WaitQueuePtr _waitQueue;
    const TraceLevelsPtr _traceLevels;
    const std::string _name;
    const NodePrx _proxy;
    const Ice::Int _waitTime;
    const std::string _instanceName;
    const UserAccountMapperPrx _userAccountMapper;
    std::string _dataDir;
    std::string _serversDir;
    std::string _tmpDir;
    unsigned long _serial;
    IceUtil::Mutex _observerMutex;
    NodeObserverPrx _observer;
    mutable PlatformInfo _platform;
    std::map<std::string, std::set<ServerIPtr> > _serversByApplication;
    std::set<std::string> _patchInProgress;
};
typedef IceUtil::Handle<NodeI> NodeIPtr;

}

#endif
