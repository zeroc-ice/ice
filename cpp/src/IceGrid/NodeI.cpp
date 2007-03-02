// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IcePatch2/Util.h>
#include <IcePatch2/ClientUtil.h>
#include <IceGrid/NodeI.h>
#include <IceGrid/Activator.h>
#include <IceGrid/ServerI.h>
#include <IceGrid/ServerAdapterI.h>
#include <IceGrid/Util.h>
#include <IceGrid/WaitQueue.h>
#include <IceGrid/TraceLevels.h>
#include <IceGrid/NodeSessionManager.h>

using namespace std;
using namespace IcePatch2;
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
    fileListProgress(Ice::Int percent)
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
    patchStart(const string& path, Ice::Long size, Ice::Long totalProgress, Ice::Long totalSize)
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
    patchProgress(Ice::Long progress, Ice::Long size, Ice::Long totalProgress, Ice::Long totalSize)
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

}

NodeI::NodeI(const Ice::ObjectAdapterPtr& adapter,
             NodeSessionManager& sessions,
             const ActivatorPtr& activator, 
             const WaitQueuePtr& waitQueue,
             const TraceLevelsPtr& traceLevels,
             const NodePrx& proxy,
             const string& name,
             const UserAccountMapperPrx& mapper) :
    _communicator(adapter->getCommunicator()),
    _adapter(adapter),
    _sessions(sessions),
    _activator(activator),
    _waitQueue(waitQueue),
    _traceLevels(traceLevels),
    _name(name),
    _proxy(proxy),
    _redirectErrToOut(false),
    _waitTime(0),
    _userAccountMapper(mapper),
    _platform("IceGrid.Node", _communicator, _traceLevels),
    _fileCache(new FileCache(_communicator)),
    _serial(1)
{
    Ice::PropertiesPtr properties = _communicator->getProperties();

    const_cast<string&>(_dataDir) = _platform.getDataDir();
    const_cast<string&>(_serversDir) = _dataDir + "/servers";
    const_cast<string&>(_tmpDir) = _dataDir + "/tmp";
    const_cast<string&>(_instanceName) = _communicator->getDefaultLocator()->ice_getIdentity().category;
    const_cast<Ice::Int&>(_waitTime) = properties->getPropertyAsIntWithDefault("IceGrid.Node.WaitTime", 60);
    const_cast<string&>(_outputDir) = properties->getProperty("IceGrid.Node.Output");
    const_cast<bool&>(_redirectErrToOut) = properties->getPropertyAsInt("IceGrid.Node.RedirectErrToOut") > 0;

    //
    // Parse the properties override property.
    //
    string props = properties->getProperty("IceGrid.Node.PropertiesOverride");
    Ice::StringSeq propsAsArgs;
    if(!props.empty())
    {
        string::size_type end = 0;
        while(end != string::npos)
        {
            const string delim = " \t\r\n";
                
            string::size_type beg = props.find_first_not_of(delim, end);
            if(beg == string::npos)
            {
                break;
            }
                
            end = props.find_first_of(delim, beg);
            string arg;
            if(end == string::npos)
            {
                arg = props.substr(beg);
            }
            else
            {
                arg = props.substr(beg, end - beg);
            }

            if(arg.find("--") == 0)
            {
                arg = arg.substr(2);
            }

            //
            // Extract the key/value
            //
            string::size_type argEnd = arg.find_first_of(delim + "=");
            if(argEnd == string::npos)
            {
                continue;
            }
            
            string key = arg.substr(0, argEnd);
            
            argEnd = arg.find('=', argEnd);
            if(argEnd == string::npos)
            {
                return;
            }
            ++argEnd;
            
            string value;
            string::size_type argBeg = arg.find_first_not_of(delim, argEnd);
            if(argBeg != string::npos)
            {
                argEnd = arg.length();
                value = arg.substr(argBeg, argEnd - argBeg);
            }

            _propertiesOverride.push_back(createProperty(key, value));
        }
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
                command = server->load(amdCB, descriptor, replicaName);
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
NodeI::destroyServer_async(const AMD_Node_destroyServerPtr& amdCB, 
                           const string& serverId, 
                           const string& uuid, 
                           int revision,
                           const string& replicaName,
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
            command = server->destroy(amdCB, uuid, revision, replicaName);
        }
        catch(const Ice::ObjectNotExistException&)
        {
        }
    }
    if(command)
    {
        command->execute();
    }
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
    if(!appDistrib->icepatch.empty())
    {
        //
        // Get all the application servers (even the ones which
        // don't have a distribution since they depend on the
        // application distribution).
        //
        servers = getApplicationServers(application);
    }
    else if(server.empty())
    {
        //
        // Get all the application servers which have a distribution.
        //
        servers = getApplicationServers(application);
        set<ServerIPtr>::iterator s = servers.begin();
        while(s != servers.end())
        {
            if((*s)->getDistribution())
            {
                ++s;
            }
            else
            {
                servers.erase(s++);
            }
        }
    }
    else
    {
        //
        // Get the given server.
        //
        Ice::Identity id = createServerIdentity(server);
        try
        {
            ServerIPtr svr = ServerIPtr::dynamicCast(_adapter->find(id));
            if(svr)
            {
                servers.insert(svr);
            }
        }
        catch(const Ice::ObjectAdapterDeactivatedException&)
        {
        }
    }

    string failure;
    if(!servers.empty())
    {
        try
        {
            set<ServerIPtr>::iterator s = servers.begin(); 
            vector<string> running;
            while(s != servers.end())
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
            
            if((servers.empty() || !appDistrib->icepatch.empty()) && !running.empty())
            {
                if(running.size() == 1)
                {
                    throw "server `" + toString(running) + "' is active";
                }
                else
                {
                    throw "servers `" + toString(running, ", ") + "' are active";
                }
            }

            for(s = servers.begin(); s != servers.end(); ++s)
            {
                (*s)->waitForPatch();
            }

            // 
            // Patch the application.
            //
            FileServerPrx icepatch;
            if(!appDistrib->icepatch.empty())
            {
                icepatch = FileServerPrx::checkedCast(_communicator->stringToProxy(appDistrib->icepatch));
                if(!icepatch)
                {
                    throw "proxy `" + appDistrib->icepatch + "' is not a file server.";
                }
                patch(icepatch, "distrib/" + application, appDistrib->directories);
            }
        
            //
            // Patch the server(s).
            //
            for(s = servers.begin(); s != servers.end(); ++s)
            {
                InternalDistributionDescriptorPtr dist = (*s)->getDistribution();
                if(!dist || (!server.empty() && (*s)->getId() != server))
                {
                    continue;
                }
            
                icepatch = FileServerPrx::checkedCast(_communicator->stringToProxy(dist->icepatch));
                if(!icepatch)
                {
                    throw "proxy `" + dist->icepatch + "' is not a file server.";
                }
                patch(icepatch, "servers/" + (*s)->getId() + "/distrib", dist->directories);
            
                if(!server.empty())
                {
                    break;
                }
            }
        }
        catch(const Ice::LocalException& e)
        {
            ostringstream os;
            os << e;
            failure = os.str();
        }
        catch(const string& e)
        {
            failure = e;
        }
        catch(const char* e)
        {
            failure = e;
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
NodeI::destroy()
{
    IceUtil::Mutex::Lock sync(_serversLock);
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

WaitQueuePtr
NodeI::getWaitQueue() const
{
    return _waitQueue;
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

NodeSessionPrx
NodeI::registerWithRegistry(const InternalRegistryPrx& registry)
{
    return registry->registerNode(_platform.getInternalNodeInfo(), _proxy, _platform.getLoadInfo());
}

void
NodeI::checkConsistency(const NodeSessionPrx& session)
{
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

    try
    {
        NodeDynamicInfo info;
        info.info = _platform.getNodeInfo();
        info.servers = serverInfos;
        info.adapters = adapterInfos;
        observer->nodeUp(info);
    }
    catch(const Ice::LocalException& ex)
    {
        Ice::Warning out(_traceLevels->logger);
        out << "unexpected observer exception:\n" << ex;
    }
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

    if(info.state == Destroyed || info.state == Inactive && info.enabled)
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
            try
            {
                p->second->updateServer(_name, info);
                sent.insert(p->second);
            }
            catch(const Ice::LocalException&)
            {
                // IGNORE
            }
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
            try
            {
                p->second->updateAdapter(_name, info);
            }
            catch(const Ice::LocalException&)
            {
                // IGNORE
            }
        }
    }
}

void
NodeI::addServer(const ServerIPtr& server, const string& application, bool dependsOnApplicationDistrib)
{
    IceUtil::Mutex::Lock sync(_serversLock);

    if(dependsOnApplicationDistrib)
    {
        map<string, set<ServerIPtr> >::iterator p = _serversByApplication.find(application);
        if(p == _serversByApplication.end())
        {
            map<string, set<ServerIPtr> >::value_type v(application, set<ServerIPtr>());
            p = _serversByApplication.insert(p, v);
        }
        p->second.insert(server);
    }
}

void
NodeI::removeServer(const ServerIPtr& server, const std::string& application, bool dependsOnApplicationDistrib)
{
    IceUtil::Mutex::Lock sync(_serversLock);
    
    if(dependsOnApplicationDistrib)
    {
        map<string, set<ServerIPtr> >::iterator p = _serversByApplication.find(application);
        if(p != _serversByApplication.end())
        {
            p->second.erase(server);
            if(p->second.empty())
            {
                _serversByApplication.erase(p);
            }
        }
    }
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
    catch(const string& msg)
    {
        Ice::Error out(_traceLevels->logger);
        out << "couldn't read directory `" << _serversDir << "':\n" << msg;
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
                    ServerCommandPtr command = server->destroy(0, "", 0, "Master");
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
                catch(const string&)
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
            catch(const string& msg)
            {
                Ice::Warning out(_traceLevels->logger);
                out << "removing server directory `" << _serversDir << "/" << *p << "' failed:\n" << msg;
            }

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
        
    if(remove.empty())
    {
        return commands;
    }

    //
    // If there's server that couldn't be removed we move them to the
    // temporary backup directory. First, we rotate the temporary
    // backup directories.
    //
    try
    {
        contents.clear();
        contents = readDirectory(_tmpDir);
    }
    catch(const string& msg)
    {
        Ice::Error out(_traceLevels->logger);
        out << "couldn't read directory `" << _tmpDir << "':\n" << msg;
        return commands;
    }

    if(contents.size() < 10)
    {
        ostringstream os;
        os << "servers-" << contents.size();
        contents.push_back(os.str());
        sort(contents.begin(), contents.end(), greater<string>());
    }
    else if(contents.size() == 10)
    {
        sort(contents.begin(), contents.end(), greater<string>());
        try
        {
            removeRecursive(_tmpDir + "/" + *contents.begin());
        }
        catch(const string& msg)
        {
            Ice::Warning out(_traceLevels->logger);
            out << msg;
        }
    }

    try
    {
        Ice::StringSeq::const_iterator p;
        for(p = contents.begin(); p != (contents.end() - 1); ++p)
        {
            rename(_tmpDir + "/" + *(p + 1), _tmpDir + "/" + *p);
        }
        createDirectoryRecursive(_tmpDir + "/servers-0");
        for(p = remove.begin(); p != remove.end(); ++p)
        {
            rename(_serversDir + "/" + *p, _tmpDir + "/servers-0/" + *p);
        }
    }
    catch(const string& msg)
    {
        Ice::Warning out(_traceLevels->logger);
        out << "rotation failed: " << msg;
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
    Ice::StringSeq contents = readDirectory(_serversDir + "/" + name);
    remove(contents.begin(), contents.end(), "dbs");
    remove(contents.begin(), contents.end(), "config");
    remove(contents.begin(), contents.end(), "distrib");
    if(!contents.empty())
    {
        return false;
    }
    
    contents = readDirectory(_serversDir + "/" + name + "/config");
    
    Ice::StringSeq::const_iterator p;
    for(p = contents.begin() ; p != contents.end(); ++p)
    {
        if(p->find("config") != 0)
        {
            return false;
        }
    }
    
    contents = readDirectory(_serversDir + "/" + name + "/dbs");
    for(p = contents.begin() ; p != contents.end(); ++p)
    {
        Ice::StringSeq files = readDirectory(_serversDir + "/" + name + "/dbs/" + *p);
        remove(files.begin(), files.end(), "DB_CONFIG");
        if(!files.empty())
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
    IcePatch2::createDirectory(_dataDir + "/" + dest);
    PatcherPtr patcher = new Patcher(icepatch, feedback, _dataDir + "/" + dest, false, 100, 1);
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

Ice::Identity
NodeI::createServerIdentity(const string& name) const
{
    Ice::Identity id;
    id.category = _instanceName + "-Server";
    id.name = name;
    return id;
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
