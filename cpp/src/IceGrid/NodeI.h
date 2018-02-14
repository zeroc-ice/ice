// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_NODE_I_H
#define ICE_GRID_NODE_I_H

#include <IceUtil/Timer.h>
#include <IcePatch2/FileServer.h>
#include <IceGrid/Internal.h>
#include <IceGrid/PlatformInfo.h>
#include <IceGrid/UserAccountMapper.h>
#include <IceGrid/FileCache.h>
#include <set>

namespace IceGrid
{

class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

class Activator;
typedef IceUtil::Handle<Activator> ActivatorPtr;

class ServerI;
typedef IceUtil::Handle<ServerI> ServerIPtr;

class ServerCommand;
typedef IceUtil::Handle<ServerCommand> ServerCommandPtr;

class NodeSessionManager;

class NodeI;
typedef IceUtil::Handle<NodeI> NodeIPtr;

class NodeI : public Node, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    class Update : virtual public IceUtil::Shared
    {
    public:
        
        Update(const NodeIPtr&, const NodeObserverPrx&);
        virtual ~Update();

        virtual bool send() = 0;

        void finished(bool);
        
        void completed(const Ice::AsyncResultPtr&);

    protected:

        const NodeIPtr _node;
        const NodeObserverPrx _observer;
    };
    typedef IceUtil::Handle<Update> UpdatePtr;

    NodeI(const Ice::ObjectAdapterPtr&, NodeSessionManager&, const ActivatorPtr&, const IceUtil::TimerPtr&, 
          const TraceLevelsPtr&, const NodePrx&, const std::string&, const UserAccountMapperPrx&, const std::string&);
    virtual ~NodeI();

    virtual void loadServer_async(const AMD_Node_loadServerPtr&, 
                                  const InternalServerDescriptorPtr&, 
                                  const std::string&,
                                  const Ice::Current&);

    virtual void loadServerWithoutRestart_async(const AMD_Node_loadServerWithoutRestartPtr&, 
                                                const InternalServerDescriptorPtr&, 
                                                const std::string&,
                                                const Ice::Current&);

    virtual void destroyServer_async(const AMD_Node_destroyServerPtr&, 
                                     const std::string&, 
                                     const std::string&,
                                     int, 
                                     const std::string&,
                                     const Ice::Current&);

    virtual void destroyServerWithoutRestart_async(const AMD_Node_destroyServerWithoutRestartPtr&, 
                                                   const std::string&, 
                                                   const std::string&,
                                                   int, 
                                                   const std::string&,
                                                   const Ice::Current&);
    
    virtual void patch_async(const AMD_Node_patchPtr&, const PatcherFeedbackPrx&, const std::string&, 
                             const std::string&, const InternalDistributionDescriptorPtr&, bool, const Ice::Current&);

    virtual void registerWithReplica(const InternalRegistryPrx&, const Ice::Current&);

    virtual void replicaInit(const InternalRegistryPrxSeq&, const Ice::Current&);
    virtual void replicaAdded(const InternalRegistryPrx&, const Ice::Current&);
    virtual void replicaRemoved(const InternalRegistryPrx&, const Ice::Current&);

    virtual std::string getName(const Ice::Current& = Ice::Current()) const;
    virtual std::string getHostname(const Ice::Current& = Ice::Current()) const;
    virtual LoadInfo getLoad(const Ice::Current& = Ice::Current()) const;
    virtual int getProcessorSocketCount(const Ice::Current&) const;
    virtual void shutdown(const Ice::Current&) const;

    virtual Ice::Long getOffsetFromEnd(const std::string&, int, const Ice::Current&) const;
    virtual bool read(const std::string&, Ice::Long, int, Ice::Long&, Ice::StringSeq&, const Ice::Current&) const;

    void shutdown();
    
    IceUtil::TimerPtr getTimer() const;
    Ice::CommunicatorPtr getCommunicator() const;
    Ice::ObjectAdapterPtr getAdapter() const;
    ActivatorPtr getActivator() const;
    TraceLevelsPtr getTraceLevels() const;
    UserAccountMapperPrx getUserAccountMapper() const;
    PlatformInfo& getPlatformInfo() const;
    FileCachePtr getFileCache() const;
    NodePrx getProxy() const;
    const PropertyDescriptorSeq& getPropertiesOverride() const;
    const std::string& getInstanceName() const;

    std::string getOutputDir() const;
    bool getRedirectErrToOut() const;
    bool allowEndpointsOverride() const;
    
    NodeSessionPrx registerWithRegistry(const InternalRegistryPrx&);
    void checkConsistency(const NodeSessionPrx&);
    NodeSessionPrx getMasterNodeSession() const;

    void addObserver(const NodeSessionPrx&, const NodeObserverPrx&);
    void removeObserver(const NodeSessionPrx&);
    void observerUpdateServer(const ServerDynamicInfo&);
    void observerUpdateAdapter(const AdapterDynamicInfo&);
    void queueUpdate(const NodeObserverPrx&, const UpdatePtr&);
    void dequeueUpdate(const NodeObserverPrx&, const UpdatePtr&, bool);

    void addServer(const ServerIPtr&, const std::string&);
    void removeServer(const ServerIPtr&, const std::string&);

    Ice::Identity createServerIdentity(const std::string&) const;
    std::string getServerAdminCategory() const;

    bool canRemoveServerDirectory(const std::string&);

private:

    std::vector<ServerCommandPtr> checkConsistencyNoSync(const Ice::StringSeq&);
    void patch(const IcePatch2::FileServerPrx&, const std::string&, const std::vector<std::string>&);
    
    std::set<ServerIPtr> getApplicationServers(const std::string&) const;
    std::string getFilePath(const std::string&) const;

    void loadServer(const AMD_Node_loadServerPtr&, const InternalServerDescriptorPtr&, const std::string&, bool,
                    const Ice::Current&);

    void destroyServer(const AMD_Node_destroyServerPtr&, const std::string&, const std::string&, int, 
                       const std::string&, bool, const Ice::Current&);

    const Ice::CommunicatorPtr _communicator;
    const Ice::ObjectAdapterPtr _adapter;
    NodeSessionManager& _sessions;
    const ActivatorPtr _activator;
    const IceUtil::TimerPtr _timer;
    const TraceLevelsPtr _traceLevels;
    const std::string _name;
    const NodePrx _proxy;
    const std::string _outputDir;
    const bool _redirectErrToOut;
    const bool _allowEndpointsOverride;
    const Ice::Int _waitTime;
    const std::string _instanceName;
    const UserAccountMapperPrx _userAccountMapper;
    mutable PlatformInfo _platform;
    const std::string _dataDir;
    const std::string _serversDir;
    const std::string _tmpDir;
    const FileCachePtr _fileCache;
    PropertyDescriptorSeq _propertiesOverride;

    unsigned long _serial;
    bool _consistencyCheckDone;

    IceUtil::Mutex _observerMutex;
    std::map<NodeSessionPrx, NodeObserverPrx> _observers;
    std::map<std::string, ServerDynamicInfo> _serversDynamicInfo;
    std::map<std::string, AdapterDynamicInfo> _adaptersDynamicInfo;

    std::map<NodeObserverPrx, std::deque<UpdatePtr> > _observerUpdates;

    IceUtil::Mutex _serversLock;
    std::map<std::string, std::set<ServerIPtr> > _serversByApplication;
    std::set<std::string> _patchInProgress;
};
typedef IceUtil::Handle<NodeI> NodeIPtr;

}

#endif
