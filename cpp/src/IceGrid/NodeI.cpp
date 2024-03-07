//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
        LogPatcherFeedback(const shared_ptr<TraceLevels>& traceLevels, const string& dest)
            : _traceLevels(traceLevels),
              _startedPatch(false),
              _lastProgress(0),
              _dest(dest)
        {
        }

        void setPatchingPath(const string& path)
        {
            _path = path;
            _startedPatch = false;
            _lastProgress = 0;
        }

        virtual bool noFileSummary(const string& reason)
        {
            if (_traceLevels->patch > 0)
            {
                Ice::Trace out(_traceLevels->logger, _traceLevels->patchCat);
                out << _dest << ": can't load summary file (will perform a thorough patch):\n" << reason;
            }
            return true;
        }

        virtual bool checksumStart()
        {
            if (_traceLevels->patch > 0)
            {
                Ice::Trace out(_traceLevels->logger, _traceLevels->patchCat);
                out << _dest << ": started checksum calculation";
            }
            return true;
        }

        virtual bool checksumProgress(const string& path)
        {
            if (_traceLevels->patch > 2)
            {
                Ice::Trace out(_traceLevels->logger, _traceLevels->patchCat);
                out << _dest << ": calculating checksum for " << getBasename(path);
            }
            return true;
        }

        virtual bool checksumEnd()
        {
            if (_traceLevels->patch > 0)
            {
                Ice::Trace out(_traceLevels->logger, _traceLevels->patchCat);
                out << _dest << ": finished checksum calculation";
            }
            return true;
        }

        virtual bool fileListStart()
        {
            if (_traceLevels->patch > 0)
            {
                Ice::Trace out(_traceLevels->logger, _traceLevels->patchCat);
                out << _dest << ": getting list of file to patch";
            }
            return true;
        }

        virtual bool fileListProgress(int32_t /*percent*/) { return true; }

        virtual bool fileListEnd()
        {
            if (_traceLevels->patch > 0)
            {
                Ice::Trace out(_traceLevels->logger, _traceLevels->patchCat);
                out << _dest << ": getting list of file to patch completed";
            }
            return true;
        }

        virtual bool patchStart(const string& /*path*/, int64_t /*size*/, int64_t totalProgress, int64_t totalSize)
        {
            if (_traceLevels->patch > 1 && totalSize > (1024 * 1024))
            {
                int progress = static_cast<int>(static_cast<double>(totalProgress) / totalSize * 100.0);
                progress /= 5;
                progress *= 5;
                if (progress != _lastProgress)
                {
                    _lastProgress = progress;
                    Ice::Trace out(_traceLevels->logger, _traceLevels->patchCat);
                    out << _dest << ": downloaded " << progress << "% (" << totalProgress << '/' << totalSize << ')';
                    if (!_path.empty())
                    {
                        out << " of " << _path;
                    }
                }
            }
            else if (_traceLevels->patch > 0)
            {
                if (!_startedPatch)
                {
                    Ice::Trace out(_traceLevels->logger, _traceLevels->patchCat);
                    int roundedSize = static_cast<int>(static_cast<double>(totalSize) / 1024);
                    if (roundedSize == 0 && totalSize > 0)
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
        patchProgress(int64_t /*progress*/, int64_t /*size*/, int64_t /*totalProgress*/, int64_t /*totalSize*/)
        {
            return true;
        }

        virtual bool patchEnd() { return true; }

        void finishPatch()
        {
            if (_traceLevels->patch > 0)
            {
                Ice::Trace out(_traceLevels->logger, _traceLevels->patchCat);
                out << _dest << ": downloading completed";
            }
        }

    private:
        const shared_ptr<TraceLevels> _traceLevels;
        bool _startedPatch;
        int _lastProgress;
        string _path;
        string _dest;
    };

}

NodeI::Update::Update(UpdateFunction updateFunction, const shared_ptr<NodeI>& node, const NodeObserverPrxPtr& observer)
    : _func(std::move(updateFunction)),
      _node(node),
      _observer(observer)
{
}

bool
NodeI::Update::send()
{
    auto self = shared_from_this();
    try
    {
        _func(
            [self] { self->_node->dequeueUpdate(self->_observer, self, false); },
            [self](exception_ptr) { self->_node->dequeueUpdate(self->_observer, self, true); });

        return true;
    }
    catch (const std::exception&)
    {
        return false;
    }
}

NodeI::NodeI(
    const shared_ptr<Ice::ObjectAdapter>& adapter,
    NodeSessionManager& sessions,
    const shared_ptr<Activator>& activator,
    const IceUtil::TimerPtr& timer,
    const shared_ptr<TraceLevels>& traceLevels,
    const NodePrxPtr& proxy,
    const string& name,
    const UserAccountMapperPrxPtr& mapper,
    const string& instanceName)
    : _communicator(adapter->getCommunicator()),
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
      _fileCache(make_shared<FileCache>(_communicator)),
      _serial(1),
      _consistencyCheckDone(false)
{
    auto props = _communicator->getProperties();

    const_cast<string&>(_dataDir) = _platform.getDataDir();
    const_cast<string&>(_serversDir) = _dataDir + "/servers";
    const_cast<string&>(_tmpDir) = _dataDir + "/tmp";
    const_cast<int&>(_waitTime) = props->getPropertyAsIntWithDefault("IceGrid.Node.WaitTime", 60);
    const_cast<string&>(_outputDir) = props->getProperty("IceGrid.Node.Output");
    const_cast<bool&>(_redirectErrToOut) = props->getPropertyAsInt("IceGrid.Node.RedirectErrToOut") > 0;
    const_cast<bool&>(_allowEndpointsOverride) = props->getPropertyAsInt("IceGrid.Node.AllowEndpointsOverride") > 0;

    //
    // Parse the properties override property.
    //
    vector<string> overrides = props->getPropertyAsList("IceGrid.Node.PropertiesOverride");
    if (!overrides.empty())
    {
        for (vector<string>::iterator p = overrides.begin(); p != overrides.end(); ++p)
        {
            if (p->find("--") != 0)
            {
                *p = "--" + *p;
            }
        }

        auto p = Ice::createProperties();
        p->parseCommandLineOptions("", overrides);
        auto propDict = p->getPropertiesForPrefix("");
        for (const auto& prop : propDict)
        {
            _propertiesOverride.push_back({prop.first, prop.second});
        }
    }
}

void
NodeI::loadServerAsync(
    shared_ptr<InternalServerDescriptor> descriptor,
    string replicaName,
    function<void(const ServerPrxPtr&, const AdapterPrxDict&, int, int)> response,
    function<void(exception_ptr)> exception,
    const Ice::Current& current)
{
    loadServer(
        std::move(descriptor), std::move(replicaName), false, std::move(response), std::move(exception), current);
}

void
NodeI::loadServerWithoutRestartAsync(
    shared_ptr<InternalServerDescriptor> descriptor,
    string replicaName,
    function<void(const ServerPrxPtr&, const AdapterPrxDict&, int, int)> response,
    function<void(exception_ptr)> exception,
    const Ice::Current& current)
{
    loadServer(std::move(descriptor), std::move(replicaName), true, std::move(response), std::move(exception), current);
}

void
NodeI::destroyServerAsync(
    string serverId,
    string uuid,
    int revision,
    string replicaName,
    function<void()> response,
    function<void(exception_ptr)> exception,
    const Ice::Current& current)
{
    destroyServer(
        std::move(serverId), std::move(uuid), std::move(revision), std::move(replicaName), false, std::move(response),
        std::move(exception), current);
}

void
NodeI::destroyServerWithoutRestartAsync(
    string serverId,
    string uuid,
    int revision,
    string replicaName,
    function<void()> response,
    function<void(exception_ptr)> exception,
    const Ice::Current& current)
{
    destroyServer(
        std::move(serverId), std::move(uuid), std::move(revision), std::move(replicaName), true, std::move(response),
        std::move(exception), current);
}

void
NodeI::patchAsync(
    PatcherFeedbackPrxPtr feedback,
    std::string application,
    std::string server,
    std::shared_ptr<InternalDistributionDescriptor> appDistrib,
    bool shutdown,
    std::function<void()> response,
    std::function<void(exception_ptr)>,
    const Ice::Current&)
{
    response();

    {
        unique_lock lock(_serversMutex);
        while (_patchInProgress.find(application) != _patchInProgress.end())
        {
            _condVar.wait(lock);
        }
        _patchInProgress.insert(application);
    }

    set<shared_ptr<ServerI>> servers;
    bool patchApplication = !appDistrib->icepatch.empty();
    if (server.empty())
    {
        //
        // Patch all the servers from the application.
        //
        servers = getApplicationServers(application);
    }
    else
    {
        shared_ptr<ServerI> svr;
        try
        {
            svr = dynamic_pointer_cast<ServerI>(_adapter->find(createServerIdentity(server)));
        }
        catch (const Ice::ObjectAdapterDeactivatedException&)
        {
        }

        if (svr)
        {
            if (appDistrib->icepatch.empty() || !svr->dependsOnApplicationDistrib())
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

    for (set<shared_ptr<ServerI>>::iterator s = servers.begin(); s != servers.end();)
    {
        if (!appDistrib->icepatch.empty() && (*s)->dependsOnApplicationDistrib())
        {
            ++s;
        }
        else if ((*s)->getDistribution() && (server.empty() || server == (*s)->getId()))
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
    if (!servers.empty())
    {
        try
        {
            vector<string> running;
            for (set<shared_ptr<ServerI>>::iterator s = servers.begin(); s != servers.end();)
            {
                try
                {
                    if (!(*s)->startPatch(shutdown))
                    {
                        running.push_back((*s)->getId());
                        servers.erase(s++);
                    }
                    else
                    {
                        ++s;
                    }
                }
                catch (const Ice::ObjectNotExistException&)
                {
                    servers.erase(s++);
                }
            }

            if (!running.empty())
            {
                if (running.size() == 1)
                {
                    throw runtime_error("server `" + toString(running) + "' is active");
                }
                else
                {
                    throw runtime_error("servers `" + toString(running, ", ") + "' are active");
                }
            }

            for (set<shared_ptr<ServerI>>::iterator s = servers.begin(); s != servers.end(); ++s)
            {
                (*s)->waitForPatch();
            }

            //
            // Patch the application.
            //
            if (patchApplication)
            {
                assert(!appDistrib->icepatch.empty());
                FileServerPrx icepatch(_communicator, appDistrib->icepatch);
                patch(icepatch, "distrib/" + application, appDistrib->directories);
            }

            //
            // Patch the server(s).
            //
            for (set<shared_ptr<ServerI>>::iterator s = servers.begin(); s != servers.end(); ++s)
            {
                InternalDistributionDescriptorPtr dist = (*s)->getDistribution();
                if (dist && (server.empty() || (*s)->getId() == server))
                {
                    FileServerPrx icepatch(_communicator, dist->icepatch);
                    patch(icepatch, "servers/" + (*s)->getId() + "/distrib", dist->directories);

                    if (!server.empty())
                    {
                        break; // No need to continue.
                    }
                }
            }
        }
        catch (const std::exception& ex)
        {
            failure = ex.what();
        }

        for (set<shared_ptr<ServerI>>::const_iterator s = servers.begin(); s != servers.end(); ++s)
        {
            (*s)->finishPatch();
        }
    }

    {
        unique_lock lock(_mutex);
        _patchInProgress.erase(application);
        _condVar.notify_all();
    }

    try
    {
        if (failure.empty())
        {
            feedback->finished();
        }
        else
        {
            feedback->failed(failure);
        }
    }
    catch (const Ice::LocalException&)
    {
    }
}

void
NodeI::registerWithReplica(InternalRegistryPrxPtr replica, const Ice::Current&)
{
    _sessions.create(std::move(replica));
}

void
NodeI::replicaInit(InternalRegistryPrxSeq replicas, const Ice::Current&)
{
    _sessions.replicaInit(std::move(replicas));
}

void
NodeI::replicaAdded(InternalRegistryPrxPtr replica, const Ice::Current&)
{
    _sessions.replicaAdded(std::move(replica));
}

void
NodeI::replicaRemoved(InternalRegistryPrxPtr replica, const Ice::Current&)
{
    _sessions.replicaRemoved(std::move(replica));
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

int64_t
NodeI::getOffsetFromEnd(string filename, int count, const Ice::Current&) const
{
    return _fileCache->getOffsetFromEnd(getFilePath(filename), count);
}

bool
NodeI::read(string filename, int64_t pos, int size, int64_t& newPos, Ice::StringSeq& lines, const Ice::Current&) const
{
    return _fileCache->read(getFilePath(filename), pos, size, newPos, lines);
}

void
NodeI::shutdown()
{
    lock_guard lock(_serversMutex);
    for (const auto& servers : _serversByApplication)
    {
        for (const auto& server : servers.second)
        {
            server->shutdown();
        }
    }
    _serversByApplication.clear();
}

shared_ptr<Ice::Communicator>
NodeI::getCommunicator() const
{
    return _communicator;
}

shared_ptr<Ice::ObjectAdapter>
NodeI::getAdapter() const
{
    return _adapter;
}

shared_ptr<Activator>
NodeI::getActivator() const
{
    return _activator;
}

IceUtil::TimerPtr
NodeI::getTimer() const
{
    return _timer;
}

shared_ptr<TraceLevels>
NodeI::getTraceLevels() const
{
    return _traceLevels;
}

UserAccountMapperPrxPtr
NodeI::getUserAccountMapper() const
{
    return _userAccountMapper;
}

PlatformInfo&
NodeI::getPlatformInfo()
{
    return _platform;
}

shared_ptr<FileCache>
NodeI::getFileCache() const
{
    return _fileCache;
}

NodePrxPtr
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

NodeSessionPrxPtr
NodeI::registerWithRegistry(const InternalRegistryPrxPtr& registry)
{
    return registry->registerNode(_platform.getInternalNodeInfo(), _proxy, _platform.getLoadInfo());
}

void
NodeI::checkConsistency(const NodeSessionPrxPtr& session)
{
    //
    // Only do the consistency check on the startup. This ensures that servers can't
    // be removed by a bogus master when the master session is re-established.
    //
    if (_consistencyCheckDone)
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
    vector<shared_ptr<ServerCommand>> commands;
    while (true)
    {
        {
            lock_guard lock(_mutex);
            if (serial == _serial)
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
        catch (const Ice::LocalException&)
        {
            return; // The connection with the session was lost.
        }
        sort(servers.begin(), servers.end());
    }

    for (const auto& command : commands)
    {
        command->execute();
    }
}

void
NodeI::addObserver(const NodeSessionPrxPtr& session, const NodeObserverPrxPtr& observer)
{
    lock_guard observerLock(_observerMutex);
    assert(_observers.find(session) == _observers.end());
    _observers.insert({session, observer});

    _observerUpdates.erase(observer); // Remove any updates from the previous session.

    ServerDynamicInfoSeq serverInfos;
    AdapterDynamicInfoSeq adapterInfos;
    for (const auto& info : _serversDynamicInfo)
    {
        assert(
            info.second.state != ServerState::Destroyed &&
            (info.second.state != ServerState::Inactive || !info.second.enabled));
        serverInfos.push_back(info.second);
    }

    for (const auto& info : _adaptersDynamicInfo)
    {
        assert(info.second.proxy);
        adapterInfos.push_back(info.second);
    }

    NodeDynamicInfo info = {_platform.getNodeInfo(), std::move(serverInfos), std::move(adapterInfos)};
    queueUpdate(
        observer, [observer, info = std::move(info)](auto&& response, auto&& exception)
        { observer->nodeUpAsync(info, std::move(response), std::move(exception)); });
}

void
NodeI::removeObserver(const NodeSessionPrxPtr& session)
{
    lock_guard observerLock(_observerMutex);
    _observers.erase(session);
}

void
NodeI::observerUpdateServer(const ServerDynamicInfo& info)
{
    lock_guard observerLock(_observerMutex);

    if (info.state == ServerState::Destroyed || (info.state == ServerState::Inactive && info.enabled))
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
    set<NodeObserverPrxPtr> sent;
    for (const auto& observer : _observers)
    {
        if (sent.find(observer.second) == sent.end())
        {
            queueUpdate(
                observer.second,
                [observer = observer.second, info, name = getName(Ice::emptyCurrent)](auto&& response, auto&& exception)
                { observer->updateServerAsync(name, info, std::move(response), std::move(exception)); });

            sent.insert(observer.second);
        }
    }
}

void
NodeI::observerUpdateAdapter(const AdapterDynamicInfo& info)
{
    lock_guard observerLock(_observerMutex);

    if (info.proxy)
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
    set<NodeObserverPrxPtr> sent;
    for (const auto& observer : _observers)
    {
        if (sent.find(observer.second) == sent.end())
        {
            queueUpdate(
                observer.second,
                [observer = observer.second, info, name = getName(Ice::emptyCurrent)](auto&& response, auto&& exception)
                { observer->updateAdapterAsync(name, info, std::move(response), std::move(exception)); });
            sent.insert(observer.second);
        }
    }
}

void
NodeI::queueUpdate(const NodeObserverPrxPtr& proxy, Update::UpdateFunction updateFunction)
{
    // Must be called with mutex locked
    auto update = make_shared<Update>(std::move(updateFunction), shared_from_this(), proxy);
    auto p = _observerUpdates.find(proxy);
    if (p == _observerUpdates.end())
    {
        if (update->send())
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
NodeI::dequeueUpdate(const NodeObserverPrxPtr& proxy, const shared_ptr<Update>& update, bool all)
{
    lock_guard observerLock(_observerMutex);
    auto p = _observerUpdates.find(proxy);
    if (p == _observerUpdates.end() || p->second.front().get() != update.get())
    {
        return;
    }

    p->second.pop_front();

    if (all || (!p->second.empty() && !p->second.front()->send()))
    {
        p->second.clear();
    }

    if (p->second.empty())
    {
        _observerUpdates.erase(p);
    }
}

void
NodeI::addServer(const shared_ptr<ServerI>& server, const string& application)
{
    lock_guard serversLock(_serversMutex);
    auto p = _serversByApplication.find(application);
    if (p == _serversByApplication.end())
    {
        p = _serversByApplication.insert(p, {application, {}});
    }
    p->second.insert(server);
}

void
NodeI::removeServer(const shared_ptr<ServerI>& server, const std::string& application)
{
    lock_guard serversLock(_serversMutex);
    auto p = _serversByApplication.find(application);
    if (p != _serversByApplication.end())
    {
        p->second.erase(server);
        if (p->second.empty())
        {
            _serversByApplication.erase(p);

            string appDir = _dataDir + "/distrib/" + application;
            if (IceUtilInternal::directoryExists(appDir))
            {
                try
                {
                    IcePatch2Internal::removeRecursive(appDir);
                }
                catch (const exception& ex)
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
    return {name, _instanceName + "-Server"};
}

string
NodeI::getServerAdminCategory() const
{
    return _instanceName + "-NodeServerAdminRouter";
}

vector<shared_ptr<ServerCommand>>
NodeI::checkConsistencyNoSync(const Ice::StringSeq& servers)
{
    vector<shared_ptr<ServerCommand>> commands;

    //
    // Check if the servers directory doesn't contain more servers
    // than the registry really knows.
    //
    Ice::StringSeq contents;
    try
    {
        contents = readDirectory(_serversDir);
    }
    catch (const exception& ex)
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
        auto p = remove.begin();
        while (p != remove.end())
        {
            auto server = dynamic_pointer_cast<ServerI>(_adapter->find(createServerIdentity(*p)));
            if (server)
            {
                //
                // If the server is loaded, we invoke on it to destroy it.
                //
                try
                {
                    auto command = server->destroy("", 0, "Master", false, nullptr);
                    if (command)
                    {
                        commands.push_back(command);
                    }
                    p = remove.erase(p);
                    continue;
                }
                catch (const Ice::LocalException& ex)
                {
                    Ice::Error out(_traceLevels->logger);
                    out << "server `" << *p << "' destroy failed:\n" << ex;
                }
                catch (const exception&)
                {
                    assert(false);
                }
            }

            try
            {
                if (canRemoveServerDirectory(*p))
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
            catch (const exception& ex)
            {
                Ice::Warning out(_traceLevels->logger);
                out << "removing server directory `" << _serversDir << "/" << *p << "' failed:\n" << ex.what();
            }

            *p = _serversDir + "/" + *p;
            ++p;
        }
    }
    catch (const Ice::ObjectAdapterDeactivatedException&)
    {
        //
        // Just return the server commands, we'll finish the
        // consistency check next time the node is started.
        //
        return commands;
    }

    if (!remove.empty())
    {
        Ice::Warning out(_traceLevels->logger);
        out << "server directories containing data not created or written by IceGrid were not removed:\n";
        out << toString(remove);
    }
    return commands;
}

NodeSessionPrxPtr
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
    contents.erase("revision");
    contents.erase("data");
    Ice::StringSeq serviceDataDirs;
    for (set<string>::const_iterator p = contents.begin(); p != contents.end(); ++p)
    {
        if (p->find("data_") != 0)
        {
            return false;
        }
        serviceDataDirs.push_back(*p);
    }
    if (!contents.empty())
    {
        return false;
    }

    c = readDirectory(_serversDir + "/" + name + "/config");
    for (Ice::StringSeq::const_iterator p = c.begin(); p != c.end(); ++p)
    {
        if (p->find("config") != 0)
        {
            return false;
        }
    }

    if (IceUtilInternal::directoryExists(_serversDir + "/" + name + "/dbs"))
    {
        c = readDirectory(_serversDir + "/" + name + "/dbs");
        for (Ice::StringSeq::const_iterator p = c.begin(); p != c.end(); ++p)
        {
            try
            {
                Ice::StringSeq files = readDirectory(_serversDir + "/" + name + "/dbs/" + *p);
                files.erase(remove(files.begin(), files.end(), "DB_CONFIG"), files.end());
                files.erase(remove(files.begin(), files.end(), "__Freeze"), files.end());
                if (!files.empty())
                {
                    return false;
                }
            }
            catch (const exception&)
            {
                return false;
            }
        }
    }

    if (IceUtilInternal::directoryExists(_serversDir + "/" + name + "/data"))
    {
        if (!readDirectory(_serversDir + "/" + name + "/data").empty())
        {
            return false;
        }
    }

    for (Ice::StringSeq::const_iterator p = serviceDataDirs.begin(); p != serviceDataDirs.end(); ++p)
    {
        try
        {
            if (!readDirectory(_serversDir + "/" + name + "/" + *p).empty())
            {
                return false;
            }
        }
        catch (const exception&)
        {
            return false;
        }
    }
    return true;
}

void
NodeI::patch(const FileServerPrx& icepatch, const string& dest, const vector<string>& directories)
{
    IcePatch2::PatcherFeedbackPtr feedback = make_shared<LogPatcherFeedback>(_traceLevels, dest);
    IcePatch2Internal::createDirectory(_dataDir + "/" + dest);
    PatcherPtr patcher = PatcherFactory::create(icepatch, feedback, _dataDir + "/" + dest, false, 100, 1);
    bool aborted = !patcher->prepare();
    if (!aborted)
    {
        if (directories.empty())
        {
            aborted = !patcher->patch("");
            dynamic_cast<LogPatcherFeedback*>(feedback.get())->finishPatch();
        }
        else
        {
            for (vector<string>::const_iterator p = directories.begin(); p != directories.end(); ++p)
            {
                dynamic_cast<LogPatcherFeedback*>(feedback.get())->setPatchingPath(*p);
                if (!patcher->patch(*p))
                {
                    aborted = true;
                    break;
                }
                dynamic_cast<LogPatcherFeedback*>(feedback.get())->finishPatch();
            }
        }
    }
    if (!aborted)
    {
        patcher->finish();
    }

    //
    // Update the files owner/group
    //
}

set<shared_ptr<ServerI>>
NodeI::getApplicationServers(const string& application) const
{
    lock_guard lock(_serversMutex);
    set<shared_ptr<ServerI>> servers;
    auto p = _serversByApplication.find(application);
    if (p != _serversByApplication.end())
    {
        servers = p->second;
    }
    return servers;
}

string
NodeI::getFilePath(const string& filename) const
{
    string file;
    if (filename == "stderr")
    {
        file = _communicator->getProperties()->getProperty("Ice.StdErr");
        if (file.empty())
        {
            throw FileNotAvailableException("Ice.StdErr configuration property is not set");
        }
    }
    else if (filename == "stdout")
    {
        file = _communicator->getProperties()->getProperty("Ice.StdOut");
        if (file.empty())
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
NodeI::loadServer(
    shared_ptr<InternalServerDescriptor> descriptor,
    string replicaName,
    bool noRestart,
    function<void(const ServerPrxPtr&, const AdapterPrxDict&, int, int)>&& response,
    function<void(exception_ptr)>&& exception,
    const Ice::Current& current)
{
    shared_ptr<ServerCommand> command;
    {
        lock_guard lock(_mutex);
        ++_serial;

        auto id = createServerIdentity(descriptor->id);

        //
        // Check if we already have a servant for this server. If that's
        // the case, the server is already loaded and we just need to
        // update it.
        //
        while (true)
        {
            bool added = false;
            shared_ptr<ServerI> server;
            try
            {
                server = dynamic_pointer_cast<ServerI>(_adapter->find(id));
                if (!server)
                {
                    auto proxy = Ice::uncheckedCast<ServerPrx>(_adapter->createProxy(id));
                    server = make_shared<ServerI>(shared_from_this(), proxy, _serversDir, descriptor->id, _waitTime);
                    _adapter->add(server, id);
                    added = true;
                }
            }
            catch (const Ice::ObjectAdapterDeactivatedException&)
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
                // Don't std::move response/exception as we may need to loop and call again load.
                command = server->load(descriptor, replicaName, noRestart, response, exception);
            }
            catch (const Ice::ObjectNotExistException&)
            {
                assert(!added);
                continue;
            }
            catch (const Ice::Exception&)
            {
                if (added)
                {
                    try
                    {
                        _adapter->remove(id);
                    }
                    catch (const Ice::ObjectAdapterDeactivatedException&)
                    {
                        // IGNORE
                    }
                }
                throw;
            }
            break;
        }
    }
    if (command)
    {
        command->execute();
    }
}

void
NodeI::destroyServer(
    string serverId,
    string uuid,
    int revision,
    string replicaName,
    bool noRestart,
    function<void()> response,
    function<void(exception_ptr)>,
    const Ice::Current& current)
{
    shared_ptr<ServerCommand> command;
    {
        lock_guard lock(_mutex);
        ++_serial;

        shared_ptr<ServerI> server;
        try
        {
            server = dynamic_pointer_cast<ServerI>(_adapter->find(createServerIdentity(serverId)));
        }
        catch (const Ice::ObjectAdapterDeactivatedException&)
        {
            //
            // We throw an object not exist exception to avoid
            // dispatch warnings. The registry will consider the node
            // has being unreachable upon receipt of this exception
            // (like any other Ice::LocalException). We could also
            // have disabled dispatch warnings but they can still
            // useful to catch other issues.
            //
            throw Ice::ObjectNotExistException(__FILE__, __LINE__, current.id, current.facet, current.operation);
        }

        if (!server)
        {
            server = make_shared<ServerI>(shared_from_this(), nullopt, _serversDir, serverId, _waitTime);
        }

        //
        // Destroy the server object if it's loaded.
        //
        try
        {
            // Don't std::move response as we may need to call it if there is an exception
            command = server->destroy(uuid, revision, replicaName, noRestart, response);
        }
        catch (const Ice::ObjectNotExistException&)
        {
            response();
            return;
        }
    }
    if (command)
    {
        command->execute();
    }
}
