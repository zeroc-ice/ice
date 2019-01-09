// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include <IceUtil/Timer.h>
#include <IceUtil/FileUtil.h>
#include <Ice/Ice.h>
#include <IcePatch2Lib/Util.h>
#include <IcePatch2/ClientUtil.h>
#include <IceGrid/NodeI.h>
#include <IceGrid/Activator.h>
#include <IceGrid/ServerI.h>
#include <IceGrid/ServerAdapterI.h>
#include <IceGrid/Util.h>
#include <IceGrid/TraceLevels.h>
#include <IceGrid/NodeSessionManager.h>

using namespace std;
using namespace IcePatch2;
using namespace IcePatch2Internal;
using namespace IceGrid;

namespace
{

class LogPatcherFeedback : public IcePatch2::PatcherFeedback
{
public:

    LogPatcherFeedback(const TraceLevelsPtr& traceLevels, const string& dest) :
        _traceLevels(traceLevels),
        _startedPatch(false),
        _lastProgress(0),
        _dest(dest)
    {
    }

    void
    setPatchingPath(const string& path)
    {
        _path = path;
        _startedPatch = false;
        _lastProgress = 0;
    }

    virtual bool
    noFileSummary(const string& reason)
    {
        if(_traceLevels->patch > 0)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->patchCat);
            out << _dest << ": can't load summary file (will perform a thorough patch):\n" << reason;
        }
        return true;
    }

    virtual bool
    checksumStart()
    {
        if(_traceLevels->patch > 0)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->patchCat);
            out << _dest << ": started checksum calculation";
        }
        return true;
    }

    virtual bool
    checksumProgress(const string& path)
    {
        if(_traceLevels->patch > 2)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->patchCat);
            out << _dest << ": calculating checksum for " << getBasename(path);
        }
        return true;
    }

    virtual bool
    checksumEnd()
    {
        if(_traceLevels->patch > 0)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->patchCat);
            out << _dest << ": finished checksum calculation";
        }
        return true;
    }

    virtual bool
    fileListStart()
    {
        if(_traceLevels->patch > 0)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->patchCat);
            out << _dest << ": getting list of file to patch";
        }
        return true;
    }

    virtual bool
    fileListProgress(Ice::Int /*percent*/)
    {
        return true;
    }

    virtual bool
    fileListEnd()
    {
        if(_traceLevels->patch > 0)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->patchCat);
            out << _dest << ": getting list of file to patch completed";
        }
        return true;
    }

    virtual bool
    patchStart(const string& /*path*/, Ice::Long /*size*/, Ice::Long totalProgress, Ice::Long totalSize)
    {
        if(_traceLevels->patch > 1 && totalSize > (1024 * 1024))
        {
            int progress = static_cast<int>(static_cast<double>(totalProgress) / totalSize * 100.0);
            progress /= 5;
            progress *= 5;
            if(progress != _lastProgress)
            {
                _lastProgress = progress;
                Ice::Trace out(_traceLevels->logger, _traceLevels->patchCat);
                out << _dest << ": downloaded " << progress << "% (" << totalProgress << '/' << totalSize << ')';
                if(!_path.empty())
                {
                    out << " of " << _path;
                }
            }
        }
        else if(_traceLevels->patch > 0)
        {
            if(!_startedPatch)
            {
                Ice::Trace out(_traceLevels->logger, _traceLevels->patchCat);
                int roundedSize = static_cast<int>(static_cast<double>(totalSize) / 1024);
                if(roundedSize == 0 && totalSize > 0)
                {
                    roundedSize = 1;
                }
                out << _dest << ": downloading " << (_path.empty() ? string("") : (_path + " ")) << roundedSize
                    << "KB ";
                _startedPatch = true;
            }
        }

        return true;
    }

    virtual bool
    patchProgress(Ice::Long /*progress*/, Ice::Long /*size*/, Ice::Long /*totalProgress*/, Ice::Long /*totalSize*/)
    {
        return true;
    }

    virtual bool
    patchEnd()
    {
        return true;
    }

    void
    finishPatch()
    {
        if(_traceLevels->patch > 0)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->patchCat);
            out << _dest << ": downloading completed";
        }
    }

private:

    const TraceLevelsPtr _traceLevels;
    bool _startedPatch;
    int _lastProgress;
    string _path;
    string _dest;
};

class NodeUp : public NodeI::Update
{
public:

    NodeUp(const NodeIPtr& node, const NodeObserverPrx& observer, NodeDynamicInfo info) :
        NodeI::Update(node, observer), _info(info)
    {
    }

    virtual bool
    send()
    {
        try
        {
            _observer->begin_nodeUp(_info, newCallback(static_cast<NodeI::Update*>(this), &NodeI::Update::completed));
        }
        catch(const Ice::LocalException&)
        {
            return false;
        }
        return true;
    }

private:

    NodeDynamicInfo _info;
};

class UpdateServer : public NodeI::Update
{
public:

    UpdateServer(const NodeIPtr& node, const NodeObserverPrx& observer, ServerDynamicInfo info) :
        NodeI::Update(node, observer), _info(info)
    {
    }

    virtual bool
    send()
    {
        try
        {
            _observer->begin_updateServer(_node->getName(Ice::emptyCurrent),
                                          _info,
                                          newCallback(static_cast<NodeI::Update*>(this), &NodeI::Update::completed));
        }
        catch(const Ice::LocalException&)
        {
            return false;
        }
        return true;
    }

private:

    ServerDynamicInfo _info;
};

class UpdateAdapter : public NodeI::Update
{
public:

    UpdateAdapter(const NodeIPtr& node, const NodeObserverPrx& observer, AdapterDynamicInfo info) :
        NodeI::Update(node, observer), _info(info)
    {
    }

    virtual bool
    send()
    {
        try
        {
            _observer->begin_updateAdapter(_node->getName(Ice::emptyCurrent),
                                           _info,
                                           newCallback(static_cast<NodeI::Update*>(this), &NodeI::Update::completed));
        }
        catch(const Ice::LocalException&)
        {
            return false;
        }
        return true;
    }

private:

    AdapterDynamicInfo _info;
};

}

NodeI::Update::Update(const NodeIPtr& node, const NodeObserverPrx& observer) : _node(node), _observer(observer)
{
}

NodeI::Update::~Update()
{
}

void
NodeI::Update::finished(bool success)
{
    _node->dequeueUpdate(_observer, this, !success);
}

NodeI::NodeI(const Ice::ObjectAdapterPtr& adapter,
             NodeSessionManager& sessions,
             const ActivatorPtr& activator,
             const IceUtil::TimerPtr& timer,
             const TraceLevelsPtr& traceLevels,
             const NodePrx& proxy,
             const string& name,
             const UserAccountMapperPrx& mapper,
             const string& instanceName) :
    _communicator(adapter->getCommunicator()),
    _adapter(adapter),
    _sessions(sessions),
    _activator(activator),
    _timer(timer),
    _traceLevels(traceLevels),
    _name(name),
    _proxy(proxy),
    _redirectErrToOut(false),
    _allowEndpointsOverride(false),
    _waitTime(0),
    _instanceName(instanceName),
    _userAccountMapper(mapper),
    _platform("IceGrid.Node", _communicator, _traceLevels),
    _fileCache(new FileCache(_communicator)),
    _serial(1),
    _consistencyCheckDone(false)
{
    Ice::PropertiesPtr props = _communicator->getProperties();

    const_cast<string&>(_dataDir) = _platform.getDataDir();
    const_cast<string&>(_serversDir) = _dataDir + "/servers";
    const_cast<string&>(_tmpDir) = _dataDir + "/tmp";
    const_cast<Ice::Int&>(_waitTime) = props->getPropertyAsIntWithDefault("IceGrid.Node.WaitTime", 60);
    const_cast<string&>(_outputDir) = props->getProperty("IceGrid.Node.Output");
    const_cast<bool&>(_redirectErrToOut) = props->getPropertyAsInt("IceGrid.Node.RedirectErrToOut") > 0;
    const_cast<bool&>(_allowEndpointsOverride) = props->getPropertyAsInt("IceGrid.Node.AllowEndpointsOverride") > 0;

    //
    // Parse the properties override property.
    //
    vector<string> overrides = props->getPropertyAsList("IceGrid.Node.PropertiesOverride");
    if(!overrides.empty())
    {
        for(vector<string>::iterator p = overrides.begin(); p != overrides.end(); ++p)
        {
            if(p->find("--") != 0)
            {
                *p = "--" + *p;
            }
        }

        Ice::PropertiesPtr p = Ice::createProperties();
        p->parseCommandLineOptions("", overrides);
        Ice::PropertyDict propDict = p->getPropertiesForPrefix("");
        for(Ice::PropertyDict::const_iterator q = propDict.begin(); q != propDict.end(); ++q)
        {
            _propertiesOverride.push_back(createProperty(q->first, q->second));
        }
    }
}

void
NodeI::Update::completed(const Ice::AsyncResultPtr& result)
{
    try
    {
        result->throwLocalException();
        finished(true);
    }
    catch(const Ice::LocalException&)
    {
        finished(false);
    }
}

NodeI::~NodeI()
{
}

void
NodeI::loadServer_async(const AMD_Node_loadServerPtr& amdCB,
                        const InternalServerDescriptorPtr& descriptor,
                        const string& replicaName,
                        const Ice::Current& current)
{
    loadServer(amdCB, descriptor, replicaName, false, current);
}

void
NodeI::loadServerWithoutRestart_async(const AMD_Node_loadServerWithoutRestartPtr& amdCB,
                                      const InternalServerDescriptorPtr& descriptor,
                                      const string& replicaName,
                                      const Ice::Current& current)
{
    class LoadServerCB : public AMD_Node_loadServer
    {
    public:

        LoadServerCB(const AMD_Node_loadServerWithoutRestartPtr& cb) : _cb(cb)
        {
        }

        virtual void
        ice_response(const ServerPrx& server, const AdapterPrxDict& adapters, Ice::Int actTimeout, Ice::Int deacTimeout)
        {
            _cb->ice_response(server, adapters, actTimeout, deacTimeout);
        };

        virtual void
        ice_exception(const ::std::exception& ex)
        {
            _cb->ice_exception(ex);
        }

        virtual void
        ice_exception()
        {
            _cb->ice_exception();
        }

    private:

        const AMD_Node_loadServerWithoutRestartPtr _cb;
    };
    loadServer(new LoadServerCB(amdCB), descriptor, replicaName, true, current);
}

void
NodeI::destroyServer_async(const AMD_Node_destroyServerPtr& amdCB,
                           const string& serverId,
                           const string& uuid,
                           int revision,
                           const string& replicaName,
                           const Ice::Current& current)
{
    destroyServer(amdCB, serverId, uuid, revision, replicaName, false, current);
}

void
NodeI::destroyServerWithoutRestart_async(const AMD_Node_destroyServerWithoutRestartPtr& amdCB,
                                         const string& serverId,
                                         const string& uuid,
                                         int revision,
                                         const string& replicaName,
                                         const Ice::Current& current)
{
    class DestroyServerCB : public AMD_Node_destroyServer
    {
    public:

        DestroyServerCB(const AMD_Node_destroyServerWithoutRestartPtr& cb) : _cb(cb)
        {
        }

        virtual void
        ice_response()
        {
            _cb->ice_response();
        };

        virtual void
        ice_exception(const ::std::exception& ex)
        {
            _cb->ice_exception(ex);
        }

        virtual void
        ice_exception()
        {
            _cb->ice_exception();
        }

    private:

        const AMD_Node_destroyServerWithoutRestartPtr _cb;
    };
    destroyServer(new DestroyServerCB(amdCB), serverId, uuid, revision, replicaName, true, current);
}

void
NodeI::patch_async(const AMD_Node_patchPtr& amdCB,
                   const PatcherFeedbackPrx& feedback,
                   const string& application,
                   const string& server,
                   const InternalDistributionDescriptorPtr& appDistrib,
                   bool shutdown,
                   const Ice::Current&)
{
    amdCB->ice_response();

    {
        Lock sync(*this);
        while(_patchInProgress.find(application) != _patchInProgress.end())
        {
            wait();
        }
        _patchInProgress.insert(application);
    }

    set<ServerIPtr> servers;
    bool patchApplication = !appDistrib->icepatch.empty();
    if(server.empty())
    {
        //
        // Patch all the servers from the application.
        //
        servers = getApplicationServers(application);
    }
    else
    {
        ServerIPtr svr;
        try
        {
            svr = ServerIPtr::dynamicCast(_adapter->find(createServerIdentity(server)));
        }
        catch(const Ice::ObjectAdapterDeactivatedException&)
        {
        }

        if(svr)
        {
            if(appDistrib->icepatch.empty() || !svr->dependsOnApplicationDistrib())
            {
                //
                // Don't patch the application if the server doesn't
                // depend on it.
                //
                patchApplication = false;
                servers.insert(svr);
            }
            else
            {
                //
                // If the server to patch depends on the application,
                // we need to shutdown all the application servers
                // that depend on the application.
                //
                servers = getApplicationServers(application);
            }
        }
    }

    for(set<ServerIPtr>::iterator s = servers.begin(); s != servers.end();)
    {
        if(!appDistrib->icepatch.empty() && (*s)->dependsOnApplicationDistrib())
        {
            ++s;
        }
        else if((*s)->getDistribution() && (server.empty() || server == (*s)->getId()))
        {
            ++s;
        }
        else
        {
            //
            // Exclude servers which don't depend on the application distribution
            // or don't have a distribution.
            //
            servers.erase(s++);
        }
    }

    string failure;
    if(!servers.empty())
    {
        try
        {
            vector<string> running;
            for(set<ServerIPtr>::iterator s = servers.begin(); s != servers.end();)
            {
                try
                {
                    if(!(*s)->startPatch(shutdown))
                    {
                        running.push_back((*s)->getId());
                        servers.erase(s++);
                    }
                    else
                    {
                        ++s;
                    }
                }
                catch(const Ice::ObjectNotExistException&)
                {
                    servers.erase(s++);
                }
            }

            if(!running.empty())
            {
                if(running.size() == 1)
                {
                    throw runtime_error("server `" + toString(running) + "' is active");
                }
                else
                {
                    throw runtime_error("servers `" + toString(running, ", ") + "' are active");
                }
            }

            for(set<ServerIPtr>::iterator s = servers.begin(); s != servers.end(); ++s)
            {
                (*s)->waitForPatch();
            }

            //
            // Patch the application.
            //
            FileServerPrx icepatch;
            if(patchApplication)
            {
                assert(!appDistrib->icepatch.empty());
                icepatch = FileServerPrx::checkedCast(_communicator->stringToProxy(appDistrib->icepatch));
                if(!icepatch)
                {
                    throw runtime_error("proxy `" + appDistrib->icepatch + "' is not a file server.");
                }
                patch(icepatch, "distrib/" + application, appDistrib->directories);
            }

            //
            // Patch the server(s).
            //
            for(set<ServerIPtr>::iterator s = servers.begin(); s != servers.end(); ++s)
            {
                InternalDistributionDescriptorPtr dist = (*s)->getDistribution();
                if(dist && (server.empty() || (*s)->getId() == server))
                {
                    icepatch = FileServerPrx::checkedCast(_communicator->stringToProxy(dist->icepatch));
                    if(!icepatch)
                    {
                        throw runtime_error("proxy `" + dist->icepatch + "' is not a file server.");
                    }
                    patch(icepatch, "servers/" + (*s)->getId() + "/distrib", dist->directories);

                    if(!server.empty())
                    {
                        break; // No need to continue.
                    }
                }
            }
        }
        catch(const exception& ex)
        {
            failure = ex.what();
        }

        for(set<ServerIPtr>::const_iterator s = servers.begin(); s != servers.end(); ++s)
        {
            (*s)->finishPatch();
        }
    }

    {
        Lock sync(*this);
        _patchInProgress.erase(application);
        notifyAll();
    }

    try
    {
        if(failure.empty())
        {
            feedback->finished();
        }
        else
        {
            feedback->failed(failure);
        }
    }
    catch(const Ice::LocalException&)
    {
    }
}

void
NodeI::registerWithReplica(const InternalRegistryPrx& replica, const Ice::Current&)
{
    _sessions.create(replica);
}

void
NodeI::replicaInit(const InternalRegistryPrxSeq& replicas, const Ice::Current&)
{
    _sessions.replicaInit(replicas);
}

void
NodeI::replicaAdded(const InternalRegistryPrx& replica, const Ice::Current&)
{
    _sessions.replicaAdded(replica);
}

void
NodeI::replicaRemoved(const InternalRegistryPrx& replica, const Ice::Current&)
{
    _sessions.replicaRemoved(replica);
}

std::string
NodeI::getName(const Ice::Current&) const
{
    return _name;
}

std::string
NodeI::getHostname(const Ice::Current&) const
{
    return _platform.getHostname();
}

LoadInfo
NodeI::getLoad(const Ice::Current&) const
{
    return _platform.getLoadInfo();
}

int
NodeI::getProcessorSocketCount(const Ice::Current&) const
{
    return _platform.getProcessorSocketCount();
}

void
NodeI::shutdown(const Ice::Current&) const
{
    _activator->shutdown();
}

Ice::Long
NodeI::getOffsetFromEnd(const string& filename, int count, const Ice::Current&) const
{
    return _fileCache->getOffsetFromEnd(getFilePath(filename), count);
}

bool
NodeI::read(const string& filename, Ice::Long pos, int size, Ice::Long& newPos, Ice::StringSeq& lines,
            const Ice::Current&) const
{
    return _fileCache->read(getFilePath(filename), pos, size, newPos, lines);
}

void
NodeI::shutdown()
{
    IceUtil::Mutex::Lock sync(_serversLock);
    for(map<string, set<ServerIPtr> >::const_iterator p = _serversByApplication.begin();
        p != _serversByApplication.end(); ++p)
    {
        for(set<ServerIPtr>::const_iterator q = p->second.begin(); q != p->second.end(); ++q)
        {
            (*q)->shutdown();
        }
    }
    _serversByApplication.clear();
}

Ice::CommunicatorPtr
NodeI::getCommunicator() const
{
    return _communicator;
}

Ice::ObjectAdapterPtr
NodeI::getAdapter() const
{
    return _adapter;
}

ActivatorPtr
NodeI::getActivator() const
{
    return _activator;
}

IceUtil::TimerPtr
NodeI::getTimer() const
{
    return _timer;
}

TraceLevelsPtr
NodeI::getTraceLevels() const
{
    return _traceLevels;
}

UserAccountMapperPrx
NodeI::getUserAccountMapper() const
{
    return _userAccountMapper;
}

PlatformInfo&
NodeI::getPlatformInfo() const
{
    return _platform;
}

FileCachePtr
NodeI::getFileCache() const
{
    return _fileCache;
}

NodePrx
NodeI::getProxy() const
{
    return _proxy;
}

const PropertyDescriptorSeq&
NodeI::getPropertiesOverride() const
{
    return _propertiesOverride;
}

const string&
NodeI::getInstanceName() const
{
    return _instanceName;
}

string
NodeI::getOutputDir() const
{
    return _outputDir;
}

bool
NodeI::getRedirectErrToOut() const
{
    return _redirectErrToOut;
}

bool
NodeI::allowEndpointsOverride() const
{
    return _allowEndpointsOverride;
}

NodeSessionPrx
NodeI::registerWithRegistry(const InternalRegistryPrx& registry)
{
    return registry->registerNode(_platform.getInternalNodeInfo(), _proxy, _platform.getLoadInfo());
}

void
NodeI::checkConsistency(const NodeSessionPrx& session)
{
    //
    // Only do the consistency check on the startup. This ensures that servers can't
    // be removed by a bogus master when the master session is re-established.
    //
    if(_consistencyCheckDone)
    {
        return;
    }
    _consistencyCheckDone = true;

    //
    // We use a serial number to keep track of the concurrent changes
    // on the node. When a server is loaded/destroyed the serial is
    // incremented. This allows to ensure that the list of servers
    // returned by the registry is consistent with the servers
    // currently deployed on the node: if the serial didn't change
    // after getting the list of servers from the registry, we have
    // the accurate list of servers that should be deployed on the
    // node.
    //
    unsigned long serial = 0;
    Ice::StringSeq servers;
    vector<ServerCommandPtr> commands;
    while(true)
    {
        {
            Lock sync(*this);
            if(serial == _serial)
            {
                _serial = 1; // We can reset the serial number.
                commands = checkConsistencyNoSync(servers);
                break;
            }
            serial = _serial;
        }
        assert(session);
        try
        {
            servers = session->getServers();
        }
        catch(const Ice::LocalException&)
        {
            return; // The connection with the session was lost.
        }
        sort(servers.begin(), servers.end());
    }

    for_each(commands.begin(), commands.end(), IceUtil::voidMemFun(&ServerCommand::execute));
}

void
NodeI::addObserver(const NodeSessionPrx& session, const NodeObserverPrx& observer)
{
    IceUtil::Mutex::Lock sync(_observerMutex);
    assert(_observers.find(session) == _observers.end());
    _observers.insert(make_pair(session, observer));

    _observerUpdates.erase(observer); // Remove any updates from the previous session.

    ServerDynamicInfoSeq serverInfos;
    AdapterDynamicInfoSeq adapterInfos;
    for(map<string, ServerDynamicInfo>::const_iterator p = _serversDynamicInfo.begin();
        p != _serversDynamicInfo.end(); ++p)
    {
        assert(p->second.state != Destroyed && (p->second.state != Inactive || !p->second.enabled));
        serverInfos.push_back(p->second);
    }

    for(map<string, AdapterDynamicInfo>::const_iterator q = _adaptersDynamicInfo.begin();
        q != _adaptersDynamicInfo.end(); ++q)
    {
        assert(q->second.proxy);
        adapterInfos.push_back(q->second);
    }

    NodeDynamicInfo info;
    info.info = _platform.getNodeInfo();
    info.servers = serverInfos;
    info.adapters = adapterInfos;
    queueUpdate(observer, new NodeUp(this, observer, info));
}

void
NodeI::removeObserver(const NodeSessionPrx& session)
{
    IceUtil::Mutex::Lock sync(_observerMutex);
    _observers.erase(session);
}

void
NodeI::observerUpdateServer(const ServerDynamicInfo& info)
{
    IceUtil::Mutex::Lock sync(_observerMutex);

    if(info.state == Destroyed || (info.state == Inactive && info.enabled))
    {
        _serversDynamicInfo.erase(info.id);
    }
    else
    {
        _serversDynamicInfo[info.id] = info;
    }

    //
    // Send the update and make sure we don't send the update twice to
    // the same observer (it's possible for the observer to be
    // registered twice if a replica is removed and added right away
    // after).
    //
    set<NodeObserverPrx> sent;
    for(map<NodeSessionPrx, NodeObserverPrx>::const_iterator p = _observers.begin(); p != _observers.end(); ++p)
    {
        if(sent.find(p->second) == sent.end())
        {
            queueUpdate(p->second, new UpdateServer(this, p->second, info));
            sent.insert(p->second);
        }
    }
}

void
NodeI::observerUpdateAdapter(const AdapterDynamicInfo& info)
{
    IceUtil::Mutex::Lock sync(_observerMutex);

    if(info.proxy)
    {
        _adaptersDynamicInfo[info.id] = info;
    }
    else
    {
        _adaptersDynamicInfo.erase(info.id);
    }

    //
    // Send the update and make sure we don't send the update twice to
    // the same observer (it's possible for the observer to be
    // registered twice if a replica is removed and added right away
    // after).
    //
    set<NodeObserverPrx> sent;
    for(map<NodeSessionPrx, NodeObserverPrx>::const_iterator p = _observers.begin(); p != _observers.end(); ++p)
    {
        if(sent.find(p->second) == sent.end())
        {
            queueUpdate(p->second, new UpdateAdapter(this, p->second, info));
            sent.insert(p->second);
        }
    }
}

void
NodeI::queueUpdate(const NodeObserverPrx& proxy, const UpdatePtr& update)
{
    //Lock sync(*this); Called within the synchronization
    map<NodeObserverPrx, deque<UpdatePtr> >::iterator p = _observerUpdates.find(proxy);
    if(p == _observerUpdates.end())
    {
        if(update->send())
        {
            _observerUpdates[proxy].push_back(update);
        }
    }
    else
    {
        p->second.push_back(update);
    }
}

void
NodeI::dequeueUpdate(const NodeObserverPrx& proxy, const UpdatePtr& update, bool all)
{
    IceUtil::Mutex::Lock sync(_observerMutex);
    map<NodeObserverPrx, deque<UpdatePtr> >::iterator p = _observerUpdates.find(proxy);
    if(p == _observerUpdates.end() || p->second.front().get() != update.get())
    {
        return;
    }

    p->second.pop_front();

    if(all || (!p->second.empty() && !p->second.front()->send()))
    {
        p->second.clear();
    }

    if(p->second.empty())
    {
        _observerUpdates.erase(p);
    }
}

void
NodeI::addServer(const ServerIPtr& server, const string& application)
{
    IceUtil::Mutex::Lock sync(_serversLock);
    map<string, set<ServerIPtr> >::iterator p = _serversByApplication.find(application);
    if(p == _serversByApplication.end())
    {
        map<string, set<ServerIPtr> >::value_type v(application, set<ServerIPtr>());
        p = _serversByApplication.insert(p, v);
    }
    p->second.insert(server);
}

void
NodeI::removeServer(const ServerIPtr& server, const std::string& application)
{
    IceUtil::Mutex::Lock sync(_serversLock);
    map<string, set<ServerIPtr> >::iterator p = _serversByApplication.find(application);
    if(p != _serversByApplication.end())
    {
        p->second.erase(server);
        if(p->second.empty())
        {
            _serversByApplication.erase(p);

            string appDir = _dataDir + "/distrib/" + application;
            if(IceUtilInternal::directoryExists(appDir))
            {
                try
                {
                    IcePatch2Internal::removeRecursive(appDir);
                }
                catch(const exception& ex)
                {
                    Ice::Warning out(_traceLevels->logger);
                    out << "removing application directory `" << appDir << "' failed:\n" << ex.what();
                }
            }
        }
    }
}

Ice::Identity
NodeI::createServerIdentity(const string& name) const
{
    Ice::Identity id;
    id.category = _instanceName + "-Server";
    id.name = name;
    return id;
}

string
NodeI::getServerAdminCategory() const
{
    return _instanceName + "-NodeServerAdminRouter";
}

vector<ServerCommandPtr>
NodeI::checkConsistencyNoSync(const Ice::StringSeq& servers)
{
    vector<ServerCommandPtr> commands;

    //
    // Check if the servers directory doesn't contain more servers
    // than the registry really knows.
    //
    Ice::StringSeq contents;
    try
    {
        contents = readDirectory(_serversDir);
    }
    catch(const exception& ex)
    {
        Ice::Error out(_traceLevels->logger);
        out << "couldn't read directory `" << _serversDir << "':\n" << ex.what();
        return commands;
    }

    vector<string> remove;
    set_difference(contents.begin(), contents.end(), servers.begin(), servers.end(), back_inserter(remove));

    //
    // Remove the extra servers if possible.
    //
    try
    {
        vector<string>::iterator p = remove.begin();
        while(p != remove.end())
        {
            ServerIPtr server = ServerIPtr::dynamicCast(_adapter->find(createServerIdentity(*p)));
            if(server)
            {
                //
                // If the server is loaded, we invoke on it to destroy it.
                //
                try
                {
                    ServerCommandPtr command = server->destroy(0, "", 0, "Master", false);
                    if(command)
                    {
                        commands.push_back(command);
                    }
                    p = remove.erase(p);
                    continue;
                }
                catch(const Ice::LocalException& ex)
                {
                    Ice::Error out(_traceLevels->logger);
                    out << "server `" << *p << "' destroy failed:\n" << ex;
                }
                catch(const exception&)
                {
                    assert(false);
                }
            }

            try
            {
                if(canRemoveServerDirectory(*p))
                {
                    //
                    // If the server directory can be removed and we
                    // either remove it or back it up before to remove it.
                    //
                    removeRecursive(_serversDir + "/" + *p);
                    p = remove.erase(p);
                    continue;
                }
            }
            catch(const exception& ex)
            {
                Ice::Warning out(_traceLevels->logger);
                out << "removing server directory `" << _serversDir << "/" << *p << "' failed:\n" << ex.what();
            }

            *p = _serversDir + "/" + *p;
            ++p;
        }
    }
    catch(const Ice::ObjectAdapterDeactivatedException&)
    {
        //
        // Just return the server commands, we'll finish the
        // consistency check next time the node is started.
        //
        return commands;
    }

    if(!remove.empty())
    {
        Ice::Warning out(_traceLevels->logger);
        out << "server directories containing data not created or written by IceGrid were not removed:\n";
        out << toString(remove);
    }
    return commands;
}

NodeSessionPrx
NodeI::getMasterNodeSession() const
{
    return _sessions.getMasterNodeSession();
}

bool
NodeI::canRemoveServerDirectory(const string& name)
{
    //
    // Check if there's files which we didn't create.
    //
    Ice::StringSeq c = readDirectory(_serversDir + "/" + name);
    set<string> contents(c.begin(), c.end());
    contents.erase("dbs");
    contents.erase("config");
    contents.erase("distrib");
    contents.erase("revision");
    contents.erase("data");
    Ice::StringSeq serviceDataDirs;
    for(set<string>::const_iterator p = contents.begin(); p != contents.end(); ++p)
    {
        if(p->find("data_") != 0)
        {
            return false;
        }
        serviceDataDirs.push_back(*p);
    }
    if(!contents.empty())
    {
        return false;
    }

    c = readDirectory(_serversDir + "/" + name + "/config");
    for(Ice::StringSeq::const_iterator p = c.begin() ; p != c.end(); ++p)
    {
        if(p->find("config") != 0)
        {
            return false;
        }
    }

    if(IceUtilInternal::directoryExists(_serversDir + "/" + name + "/dbs"))
    {
        c = readDirectory(_serversDir + "/" + name + "/dbs");
        for(Ice::StringSeq::const_iterator p = c.begin() ; p != c.end(); ++p)
        {
            try
            {
                Ice::StringSeq files = readDirectory(_serversDir + "/" + name + "/dbs/" + *p);
                files.erase(remove(files.begin(), files.end(), "DB_CONFIG"), files.end());
                files.erase(remove(files.begin(), files.end(), "__Freeze"), files.end());
                if(!files.empty())
                {
                    return false;
                }
            }
            catch(const exception&)
            {
                return false;
            }
        }
    }

    if(IceUtilInternal::directoryExists(_serversDir + "/" + name + "/data"))
    {
        if(!readDirectory(_serversDir + "/" + name + "/data").empty())
        {
            return false;
        }
    }

    for(Ice::StringSeq::const_iterator p = serviceDataDirs.begin(); p != serviceDataDirs.end(); ++p)
    {
        try
        {
            if(!readDirectory(_serversDir + "/" + name + "/" + *p).empty())
            {
                return false;
            }
        }
        catch(const exception&)
        {
            return false;
        }
    }
    return true;
}

void
NodeI::patch(const FileServerPrx& icepatch, const string& dest, const vector<string>& directories)
{
    IcePatch2::PatcherFeedbackPtr feedback = new LogPatcherFeedback(_traceLevels, dest);
    IcePatch2Internal::createDirectory(_dataDir + "/" + dest);
    PatcherPtr patcher = PatcherFactory::create(icepatch, feedback, _dataDir + "/" + dest, false, 100, 1);
    bool aborted = !patcher->prepare();
    if(!aborted)
    {
        if(directories.empty())
        {
            aborted = !patcher->patch("");
            dynamic_cast<LogPatcherFeedback*>(feedback.get())->finishPatch();
        }
        else
        {
            for(vector<string>::const_iterator p = directories.begin(); p != directories.end(); ++p)
            {
                dynamic_cast<LogPatcherFeedback*>(feedback.get())->setPatchingPath(*p);
                if(!patcher->patch(*p))
                {
                    aborted = true;
                    break;
                }
                dynamic_cast<LogPatcherFeedback*>(feedback.get())->finishPatch();
            }
        }
    }
    if(!aborted)
    {
        patcher->finish();
    }

    //
    // Update the files owner/group
    //
}

set<ServerIPtr>
NodeI::getApplicationServers(const string& application) const
{
    IceUtil::Mutex::Lock sync(_serversLock);
    set<ServerIPtr> servers;
    map<string, set<ServerIPtr> >::const_iterator p = _serversByApplication.find(application);
    if(p != _serversByApplication.end())
    {
        servers = p->second;
    }
    return servers;
}

string
NodeI::getFilePath(const string& filename) const
{
    string file;
    if(filename == "stderr")
    {
        file = _communicator->getProperties()->getProperty("Ice.StdErr");
        if(file.empty())
        {
            throw FileNotAvailableException("Ice.StdErr configuration property is not set");
        }
    }
    else if(filename == "stdout")
    {
        file = _communicator->getProperties()->getProperty("Ice.StdOut");
        if(file.empty())
        {
            throw FileNotAvailableException("Ice.StdOut configuration property is not set");
        }
    }
    else
    {
        throw FileNotAvailableException("unknown file");
    }
    return file;
}

void
NodeI::loadServer(const AMD_Node_loadServerPtr& amdCB,
                  const InternalServerDescriptorPtr& descriptor,
                  const string& replicaName,
                  bool noRestart,
                  const Ice::Current& current)
{
    ServerCommandPtr command;
    {
        Lock sync(*this);
        ++_serial;

        Ice::Identity id = createServerIdentity(descriptor->id);

        //
        // Check if we already have a servant for this server. If that's
        // the case, the server is already loaded and we just need to
        // update it.
        //
        while(true)
        {
            bool added = false;
            ServerIPtr server;
            try
            {
                server = ServerIPtr::dynamicCast(_adapter->find(id));
                if(!server)
                {
                    ServerPrx proxy = ServerPrx::uncheckedCast(_adapter->createProxy(id));
                    server = new ServerI(this, proxy, _serversDir, descriptor->id, _waitTime);
                    _adapter->add(server, id);
                    added = true;
                }
            }
            catch(const Ice::ObjectAdapterDeactivatedException&)
            {
                //
                // We throw an object not exist exception to avoid
                // dispatch warnings. The registry will consider the
                // node has being unreachable upon receival of this
                // exception (like any other Ice::LocalException). We
                // could also have disabled dispatch warnings but they
                // can still useful to catch other issues.
                //
                throw Ice::ObjectNotExistException(__FILE__, __LINE__, current.id, current.facet, current.operation);
            }

            try
            {
                command = server->load(amdCB, descriptor, replicaName, noRestart);
            }
            catch(const Ice::ObjectNotExistException&)
            {
                assert(!added);
                continue;
            }
            catch(const Ice::Exception&)
            {
                if(added)
                {
                    try
                    {
                        _adapter->remove(id);
                    }
                    catch(const Ice::ObjectAdapterDeactivatedException&)
                    {
                        // IGNORE
                    }
                }
                throw;
            }
            break;
        }
    }
    if(command)
    {
        command->execute();
    }
}

void
NodeI::destroyServer(const AMD_Node_destroyServerPtr& amdCB,
                     const string& serverId,
                     const string& uuid,
                     int revision,
                     const string& replicaName,
                     bool noRestart,
                     const Ice::Current& current)
{
    ServerCommandPtr command;
    {
        Lock sync(*this);
        ++_serial;

        ServerIPtr server;
        try
        {
            server = ServerIPtr::dynamicCast(_adapter->find(createServerIdentity(serverId)));
        }
        catch(const Ice::ObjectAdapterDeactivatedException&)
        {
            //
            // We throw an object not exist exception to avoid
            // dispatch warnings. The registry will consider the node
            // has being unreachable upon receival of this exception
            // (like any other Ice::LocalException). We could also
            // have disabled dispatch warnings but they can still
            // useful to catch other issues.
            //
            throw Ice::ObjectNotExistException(__FILE__, __LINE__, current.id, current.facet, current.operation);
        }

        if(!server)
        {
            server = new ServerI(this, 0, _serversDir, serverId, _waitTime);
        }

        //
        // Destroy the server object if it's loaded.
        //
        try
        {
            command = server->destroy(amdCB, uuid, revision, replicaName, noRestart);
        }
        catch(const Ice::ObjectNotExistException&)
        {
            amdCB->ice_response();
            return;
        }
    }
    if(command)
    {
        command->execute();
    }
}
