// Copyright (c) ZeroC, Inc.

#include "NodeI.h"
#include "../Ice/FileUtil.h"
#include "../Ice/Timer.h"
#include "Activator.h"
#include "Ice/Ice.h"
#include "NodeSessionManager.h"
#include "ServerAdapterI.h"
#include "ServerI.h"
#include "TraceLevels.h"
#include "Util.h"

using namespace std;
using namespace IceGrid;

NodeI::Update::Update(
    UpdateFunction updateFunction,
    const shared_ptr<NodeI>& node,
    const optional<NodeObserverPrx>& observer)
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
    const Ice::ObjectAdapterPtr& adapter,
    NodeSessionManager& sessions,
    const shared_ptr<Activator>& activator,
    IceInternal::TimerPtr timer,
    const shared_ptr<TraceLevels>& traceLevels,
    NodePrx proxy,
    string name,
    const optional<UserAccountMapperPrx>& mapper,
    string instanceName)
    : _communicator(adapter->getCommunicator()),
      _adapter(adapter),
      _sessions(sessions),
      _activator(activator),
      _timer(std::move(timer)),
      _traceLevels(traceLevels),
      _name(std::move(name)),
      _proxy(std::move(proxy)),
      _instanceName(std::move(instanceName)),
      _userAccountMapper(mapper),
      _platform("IceGrid.Node", _communicator, _traceLevels),
      _fileCache(make_shared<FileCache>(_communicator))
{
    auto props = _communicator->getProperties();

    const_cast<string&>(_dataDir) = _platform.getDataDir();
    const_cast<string&>(_serversDir) = _dataDir + "/servers";
    const_cast<string&>(_tmpDir) = _dataDir + "/tmp";
    const_cast<int&>(_waitTime) = props->getIcePropertyAsInt("IceGrid.Node.WaitTime");
    const_cast<string&>(_outputDir) = props->getIceProperty("IceGrid.Node.Output");
    const_cast<bool&>(_redirectErrToOut) = props->getIcePropertyAsInt("IceGrid.Node.RedirectErrToOut") > 0;
    const_cast<bool&>(_allowEndpointsOverride) = props->getIcePropertyAsInt("IceGrid.Node.AllowEndpointsOverride") > 0;

    //
    // Parse the properties override property.
    //
    vector<string> overrides = props->getIcePropertyAsList("IceGrid.Node.PropertiesOverride");
    if (!overrides.empty())
    {
        for (auto& override : overrides)
        {
            if (override.find("--") != 0)
            {
                override = "--" + override;
            }
        }

        auto parsedOptions = Ice::Properties::parseOptions("", overrides);

        // Since we're using an empty prefix, all properties will be matched
        assert(parsedOptions.second.empty());

        for (const auto& prop : parsedOptions.first)
        {
            _propertiesOverride.push_back({prop.first, prop.second});
        }
    }
}

void
NodeI::loadServerAsync(
    shared_ptr<InternalServerDescriptor> descriptor,
    string replicaName,
    function<void(const optional<ServerPrx>&, const AdapterPrxDict&, int, int)> response,
    function<void(exception_ptr)> exception,
    const Ice::Current& current)
{
    loadServer(
        std::move(descriptor),
        std::move(replicaName),
        false,
        std::move(response),
        std::move(exception),
        current);
}

void
NodeI::loadServerWithoutRestartAsync(
    shared_ptr<InternalServerDescriptor> descriptor,
    string replicaName,
    function<void(const optional<ServerPrx>&, const AdapterPrxDict&, int, int)> response,
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
        std::move(serverId),
        std::move(uuid),
        revision,
        std::move(replicaName),
        false,
        std::move(response),
        std::move(exception),
        current);
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
        std::move(serverId),
        std::move(uuid),
        revision,
        std::move(replicaName),
        true,
        std::move(response),
        std::move(exception),
        current);
}

void
NodeI::registerWithReplica(std::optional<InternalRegistryPrx> replica, const Ice::Current& current)
{
    Ice::checkNotNull(replica, __FILE__, __LINE__, current);
    _sessions.create(std::move(*replica));
}

void
NodeI::replicaInit(InternalRegistryPrxSeq replicas, const Ice::Current& current)
{
    _sessions.replicaInit(replicas, current);
}

void
NodeI::replicaAdded(optional<InternalRegistryPrx> replica, const Ice::Current& current)
{
    Ice::checkNotNull(replica, __FILE__, __LINE__, current);
    _sessions.replicaAdded(std::move(*replica));
}

void
NodeI::replicaRemoved(optional<InternalRegistryPrx> replica, const Ice::Current& current)
{
    Ice::checkNotNull(replica, __FILE__, __LINE__, current);
    _sessions.replicaRemoved(*replica);
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

Ice::ObjectAdapterPtr
NodeI::getAdapter() const
{
    return _adapter;
}

shared_ptr<Activator>
NodeI::getActivator() const
{
    return _activator;
}

IceInternal::TimerPtr
NodeI::getTimer() const
{
    return _timer;
}

shared_ptr<TraceLevels>
NodeI::getTraceLevels() const
{
    return _traceLevels;
}

optional<UserAccountMapperPrx>
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

optional<NodeSessionPrx>
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
NodeI::addObserver(NodeSessionPrx session, NodeObserverPrx observer)
{
    lock_guard observerLock(_observerMutex);
    assert(_observers.find(session) == _observers.end());
    _observers.insert({std::move(session), observer});

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
        observer,
        [observer, info = std::move(info)](auto&& response, auto&& exception)
        { observer->nodeUpAsync(info, std::move(response), std::move(exception)); });
}

void
NodeI::removeObserver(const NodeSessionPrx& session)
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
    set<optional<NodeObserverPrx>> sent;
    for (const auto& observer : _observers)
    {
        if (sent.find(observer.second) == sent.end())
        {
            queueUpdate(
                observer.second,
                [observer = observer.second, info, name = _name](auto&& response, auto&& exception)
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
    set<optional<NodeObserverPrx>> sent;
    for (const auto& observer : _observers)
    {
        if (sent.find(observer.second) == sent.end())
        {
            queueUpdate(
                observer.second,
                [observer = observer.second, info, name = _name](auto&& response, auto&& exception)
                { observer->updateAdapterAsync(name, info, std::move(response), std::move(exception)); });
            sent.insert(observer.second);
        }
    }
}

void
NodeI::queueUpdate(const optional<NodeObserverPrx>& proxy, Update::UpdateFunction updateFunction)
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
NodeI::dequeueUpdate(const optional<NodeObserverPrx>& proxy, const shared_ptr<Update>& update, bool all)
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
        out << "couldn't read directory '" << _serversDir << "':\n" << ex.what();
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
                    out << "server '" << *p << "' destroy failed:\n" << ex;
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
                out << "removing server directory '" << _serversDir << "/" << *p << "' failed:\n" << ex.what();
            }

            *p = _serversDir + "/" + *p;
            ++p;
        }
    }
    catch (const Ice::ObjectAdapterDestroyedException&)
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

optional<NodeSessionPrx>
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
    contents.erase("config");
    contents.erase("revision");
    contents.erase("data");
    Ice::StringSeq serviceDataDirs;
    for (const auto& content : contents)
    {
        if (content.find("data_") != 0)
        {
            return false;
        }
        serviceDataDirs.push_back(content);
    }
    if (!contents.empty())
    {
        return false;
    }

    c = readDirectory(_serversDir + "/" + name + "/config");
    for (const auto& p : c)
    {
        if (p.find("config") != 0)
        {
            return false;
        }
    }

    if (IceInternal::directoryExists(_serversDir + "/" + name + "/data"))
    {
        if (!readDirectory(_serversDir + "/" + name + "/data").empty())
        {
            return false;
        }
    }

    for (const auto& serviceDataDir : serviceDataDirs)
    {
        try
        {
            if (!readDirectory(_serversDir + "/" + name + "/" + serviceDataDir).empty())
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
        file = _communicator->getProperties()->getIceProperty("Ice.StdErr");
        if (file.empty())
        {
            throw FileNotAvailableException("Ice.StdErr configuration property is not set");
        }
    }
    else if (filename == "stdout")
    {
        file = _communicator->getProperties()->getIceProperty("Ice.StdOut");
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
    shared_ptr<InternalServerDescriptor> descriptor, // NOLINT(performance-unnecessary-value-param)
    string replicaName,                              // NOLINT(performance-unnecessary-value-param)
    bool noRestart,
    function<void(const optional<ServerPrx>&, const AdapterPrxDict&, int, int)>
        response,                            // NOLINT(performance-unnecessary-value-param)
    function<void(exception_ptr)> exception, // NOLINT(performance-unnecessary-value-param)
    const Ice::Current&)
{
    shared_ptr<ServerCommand> command;
    {
        lock_guard lock(_mutex);
        ++_serial;

        auto id = createServerIdentity(descriptor->id);

        // Check if we already have a servant for this server. If that's the case, the server is already loaded and we
        // just need to update it.
        while (true)
        {
            bool added = false;
            shared_ptr<ServerI> server;
            try
            {
                server = dynamic_pointer_cast<ServerI>(_adapter->find(id));
                if (!server)
                {
                    server = make_shared<ServerI>(
                        shared_from_this(),
                        _adapter->createProxy<ServerPrx>(id),
                        _serversDir,
                        descriptor->id,
                        _waitTime);
                    _adapter->add(server, id);
                    added = true;
                }
            }
            catch (const Ice::ObjectAdapterDestroyedException&)
            {
                // We throw an object not exist exception to avoid dispatch warnings. The registry will consider the
                // node has being unreachable upon receipt of this exception (like any other Ice::LocalException). We
                // could also have disabled dispatch warnings but they can still useful to catch other issues.
                throw Ice::ObjectNotExistException{__FILE__, __LINE__};
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
                    catch (const Ice::ObjectAdapterDestroyedException&)
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
    string serverId, // NOLINT(performance-unnecessary-value-param)
    string uuid,     // NOLINT(performance-unnecessary-value-param)
    int revision,
    string replicaName, // NOLINT(performance-unnecessary-value-param)
    bool noRestart,
    function<void()> response,     // NOLINT(performance-unnecessary-value-param)
    function<void(exception_ptr)>, // NOLINT(performance-unnecessary-value-param)
    const Ice::Current&)
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
        catch (const Ice::ObjectAdapterDestroyedException&)
        {
            // We throw an object not exist exception to avoid dispatch warnings. The registry will consider the node
            // has being unreachable upon receipt of this exception (like any other Ice::LocalException). We could also
            // have disabled dispatch warnings but they can still useful to catch other issues.
            throw Ice::ObjectNotExistException{__FILE__, __LINE__};
        }

        if (!server)
        {
            server = make_shared<ServerI>(shared_from_this(), nullopt, _serversDir, serverId, _waitTime);
        }

        // Destroy the server object if it's loaded.
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
