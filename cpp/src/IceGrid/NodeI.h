// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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

#include <IcePatch2/FileServer.h>

namespace IceGrid
{

class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

class Activator;
typedef IceUtil::Handle<Activator> ActivatorPtr;

class ServerI;
typedef IceUtil::Handle<ServerI> ServerIPtr;

class NodeI : public Node, public IceUtil::Mutex
{
public:

    NodeI(const Ice::ObjectAdapterPtr&, const ActivatorPtr&, const WaitQueuePtr&, const TraceLevelsPtr&, 
	  const NodePrx&, const std::string&);
    virtual ~NodeI();

    virtual ServerPrx loadServer(const std::string&, const ServerDescriptorPtr&, AdapterPrxDict&, int&, int&, 
				 const Ice::Current&);
    virtual void destroyServer(const std::string&, const Ice::Current&);
    virtual void patch(const std::string&, const DistributionDescriptor&, const DistributionDescriptorDict&, 
		       bool, const Ice::Current&);
    virtual std::string getName(const Ice::Current& = Ice::Current()) const;
    virtual std::string getHostname(const Ice::Current& = Ice::Current()) const;
    virtual void shutdown(const Ice::Current&) const;
    
    WaitQueuePtr getWaitQueue() const;
    Ice::CommunicatorPtr getCommunicator() const;
    ActivatorPtr getActivator() const;
    TraceLevelsPtr getTraceLevels() const;
    NodeObserverPrx getObserver() const;

    NodeSessionPrx getSession() const;
    void setSession(const NodeSessionPrx&, const NodeObserverPrx&);
    void keepAlive();
    void stop();

private:

    void checkConsistency();
    void checkConsistencyNoSync(const Ice::StringSeq&);
    bool canRemoveServerDirectory(const std::string&);
    void initObserver(const Ice::StringSeq&);
    void patch(const IcePatch2::FileServerPrx&, const std::string&, const std::vector<std::string>&);

    void addServer(const ServerIPtr&);
    void removeServer(const ServerIPtr&);
    std::set<ServerIPtr> getApplicationServers(const std::string&);

    const Ice::ObjectAdapterPtr _adapter;
    const ActivatorPtr _activator;
    const WaitQueuePtr _waitQueue;
    const TraceLevelsPtr _traceLevels;
    const std::string _name;
    const NodePrx _proxy;
    const Ice::Int _waitTime;
    std::string _dataDir;
    std::string _serversDir;
    std::string _tmpDir;
    unsigned long _serial;
    NodeObserverPrx _observer;
    IceUtil::Mutex _sessionMutex;
    NodeSessionPrx _session;
    PlatformInfo _platform;
    std::map<std::string, std::set<ServerIPtr> > _serversByApplication;
};
typedef IceUtil::Handle<NodeI> NodeIPtr;

}

#endif
