//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Communicator.h>
#include <Ice/Properties.h>
#include <Ice/LoggerUtil.h>
#include <IceGrid/NodeCache.h>
#include <IceGrid/SessionI.h>
#include <IceGrid/NodeSessionI.h>
#include <IceGrid/ServerCache.h>
#include <IceGrid/ReplicaCache.h>
#include <IceGrid/DescriptorHelper.h>

using namespace std;
using namespace IceGrid;

namespace
{

PropertyDescriptor
removeProperty(PropertyDescriptorSeq& properties, const string& name)
{
    string value;
    PropertyDescriptorSeq::iterator p = properties.begin();
    while(p != properties.end())
    {
        if(p->name == name)
        {
            value = p->value;
            p = properties.erase(p);
        }
        else
        {
            ++p;
        }
    }
    return { name, value };
}

}

NodeCache::NodeCache(const shared_ptr<Ice::Communicator>& communicator,
                     ReplicaCache& replicaCache, const string& replicaName) :
    _communicator(communicator),
    _replicaName(replicaName),
    _replicaCache(replicaCache)
{
}

shared_ptr<NodeEntry>
NodeCache::get(const string& name, bool create) const
{
    lock_guard lock(_mutex);

    auto cacheEntry = getImpl(name);
    if(!cacheEntry && create)
    {
        NodeCache& self = const_cast<NodeCache&>(*this);
        cacheEntry = make_shared<NodeEntry>(self, name);
        self.addImpl(name, cacheEntry);
    }
    if(!cacheEntry)
    {
        throw NodeNotExistException(name);
    }

    // Get a self removing shared_ptr to the cached NodeEntry which will remove
    // itself from the this cache upon destruction
    auto entry = cacheEntry->_selfRemovingPtr.lock();

    if (!entry)
    {
        // Create self removing shared_ptr of cacheEntry. The cacheEntry maintains a ref count for the case where
        // the self removing shared_ptr has no more references but its deleter has yet to run (weak_ptr has expired)
        // and at the same time another thread calls NodeCache::get which refreshes the self removing ptr before
        // the cached entry can be removed.
        entry = shared_ptr<NodeEntry>(const_cast<NodeEntry*>(cacheEntry.get()),
            [cache = const_cast<NodeCache*>(this), name](NodeEntry* e)
            {
                lock_guard cacheLock(cache->_mutex);
                if(--e->_selfRemovingRefCount == 0)
                {
                    cache->removeImpl(name);
                }
            });
        cacheEntry->_selfRemovingRefCount++;
        cacheEntry->_selfRemovingPtr = entry;
    }

    return entry;
}

NodeEntry::NodeEntry(NodeCache& cache, const std::string& name) :
    _cache(cache),
    _name(name),
    _registering(false),
    _selfRemovingRefCount(0)
{
}

void
NodeEntry::addDescriptor(const string& application, const NodeDescriptor& descriptor)
{
    lock_guard lock(_mutex);
    _descriptors.insert(make_pair(application, descriptor));
}

void
NodeEntry::removeDescriptor(const string& application)
{
    lock_guard lock(_mutex);
    _descriptors.erase(application);
}

void
NodeEntry::addServer(const shared_ptr<ServerEntry>& entry)
{
    lock_guard lock(_mutex);
    _servers.insert(make_pair(entry->getId(), entry));
}

void
NodeEntry::removeServer(const shared_ptr<ServerEntry>& entry)
{
    lock_guard lock(_mutex);
    _servers.erase(entry->getId());
}

void
NodeEntry::setSession(const shared_ptr<NodeSessionI>& session)
{
    unique_lock lock(_mutex);

    if(session)
    {
        while(_session)
        {
            if(_session->isDestroyed())
            {
                // If the current session has just been destroyed, wait for the setSession(0) call.
                assert(session != _session);
                _condVar.wait(lock);
            }
            else
            {
                auto s = _session;
                lock.unlock();
                try
                {
                    s->getNode()->ice_ping();
                    throw NodeActiveException();
                }
                catch(const Ice::LocalException&)
                {
                    try
                    {
                        s->destroy(Ice::emptyCurrent);
                    }
                    catch(const Ice::ObjectNotExistException&)
                    {
                    }
                }
                lock.lock();
            }
        }

        //
        // Clear the saved proxy, the node has established a session
        // so we won't need anymore to try to register it with this
        // registry.
        //
        _proxy = nullptr;
    }
    else
    {
        if(!_session)
        {
            return;
        }
    }

    _session = session;
    _condVar.notify_all();

    if(_registering)
    {
        _registering = false;
        _condVar.notify_all();
    }

    if(session)
    {
        if(_cache.getTraceLevels() && _cache.getTraceLevels()->node > 0)
        {
            Ice::Trace out(_cache.getTraceLevels()->logger, _cache.getTraceLevels()->nodeCat);
            out << "node `" << _name << "' up";
        }
    }
    else
    {
        if(_cache.getTraceLevels() && _cache.getTraceLevels()->node > 0)
        {
            Ice::Trace out(_cache.getTraceLevels()->logger, _cache.getTraceLevels()->nodeCat);
            out << "node `" << _name << "' down";
        }
    }
}

shared_ptr<NodePrx>
NodeEntry::getProxy() const
{
    unique_lock lock(_mutex);
    checkSession(lock);
    return _session->getNode();
}

shared_ptr<InternalNodeInfo>
NodeEntry::getInfo() const
{
    unique_lock lock(_mutex);
    checkSession(lock);
    return _session->getInfo();
}

ServerEntrySeq
NodeEntry::getServers() const
{
    lock_guard lock(_mutex);
    ServerEntrySeq entries;
    for(map<string, shared_ptr<ServerEntry>>::const_iterator p = _servers.begin(); p != _servers.end(); ++p)
    {
        entries.push_back(p->second);
    }
    return entries;
}

LoadInfo
NodeEntry::getLoadInfoAndLoadFactor(const string& application, float& loadFactor) const
{
    unique_lock lock(_mutex);
    checkSession(lock);

    map<string, NodeDescriptor>::const_iterator p = _descriptors.find(application);
    if(p == _descriptors.end())
    {
        throw NodeNotExistException(); // The node doesn't exist in the given application.
    }

    //
    // TODO: Cache the load factors? Parsing the load factor for each
    // call could be costly.
    //
    loadFactor = -1.0f;
    if(!p->second.loadFactor.empty())
    {
        istringstream is(p->second.loadFactor);
        is >> loadFactor;
    }
    if(loadFactor < 0.0f)
    {
        if(_session->getInfo()->os != "Windows")
        {
            //
            // On Unix platforms, we divide the load averages by the
            // number of processors. A load of 2 on a dual processor
            // machine is the same as a load of 1 on a single process
            // machine.
            //
            loadFactor = 1.0f / _session->getInfo()->nProcessors;
        }
        else
        {
            //
            // On Windows, load1, load5 and load15 are the average of
            // the CPU utilization (all CPUs). We don't need to divide
            // by the number of CPU.
            //
            loadFactor = 1.0f;
        }
    }

    return _session->getLoadInfo();
}

shared_ptr<NodeSessionI>
NodeEntry::getSession() const
{
    unique_lock lock(_mutex);
    checkSession(lock);
    return _session;
}

shared_ptr<Ice::ObjectPrx>
NodeEntry::getAdminProxy() const
{
    auto prx = getProxy();
    assert(prx);
    return prx->ice_identity({ "NodeAdmin-" + _name, prx->ice_getIdentity().category });
}

bool
NodeEntry::canRemove()
{
    lock_guard lock(_mutex);

    // The cache mutex must be locked to acesss _selfRemovingRefCount
    return _servers.empty() && !_session && _descriptors.empty() && _selfRemovingRefCount == 0;
}

void
NodeEntry::loadServer(const shared_ptr<ServerEntry>& entry, const ServerInfo& server,
                      const shared_ptr<SessionI>& session, chrono::seconds timeout, bool noRestart)
{
    try
    {
        shared_ptr<NodePrx> node;
        chrono::seconds sessionTimeout;
        shared_ptr<InternalServerDescriptor> desc;
        {
            unique_lock lock(_mutex);
            checkSession(lock);
            node = _session->getNode();
            sessionTimeout = chrono::seconds(_session->getTimeout(Ice::emptyCurrent));

            //
            // Check if we should use a specific timeout (the load
            // call can deactivate the server and it can take some
            // time to deactivate, up to "deactivation-timeout"
            // seconds).
            //
            if(timeout > 0s)
            {
                auto timeoutInMilliseconds = secondsToInt(timeout) * 1000;
                node = Ice::uncheckedCast<NodePrx>(node->ice_invocationTimeout(move(timeoutInMilliseconds)));
            }

            ServerInfo info = server;
            try
            {
                info.descriptor = getServerDescriptor(server, session);
            }
            catch(const DeploymentException&)
            {
                //
                // We ignore the deployment error for now (which can
                // only be caused in theory by session variables not
                // being defined because the server isn't
                // allocated...)
                //
            }
            desc = getInternalServerDescriptor(info);
        }
        assert(desc);

        if(_cache.getTraceLevels() && _cache.getTraceLevels()->server > 2)
        {
            Ice::Trace out(_cache.getTraceLevels()->logger, _cache.getTraceLevels()->serverCat);
            out << "loading `" << desc->id << "' on node `" << _name << "'";
            if(session)
            {
                out << " for session `" << session->getId() << "'";
            }
        }

        auto response = [traceLevels = _cache.getTraceLevels(), entry, name = _name, sessionTimeout]
            (shared_ptr<ServerPrx> serverPrx, AdapterPrxDict adapters, int at, int dt)
            {
                if(traceLevels && traceLevels->server > 1)
                {
                    Ice::Trace out(traceLevels->logger, traceLevels->serverCat);
                    out << "loaded `" << entry->getId() << "' on node `" << name << "'";
                }

                //
                // Add the node session timeout on the proxies to ensure the
                // timeout is large enough.
                //
                entry->loadCallback(move(serverPrx), move(adapters),
                                    chrono::seconds(at) + sessionTimeout,
                                    chrono::seconds(dt) + sessionTimeout);

            };

        auto exception = [traceLevels = _cache.getTraceLevels(), entry, name = _name](auto exptr)
            {
                try
                {
                    rethrow_exception(exptr);
                }
                catch(const DeploymentException& ex)
                {
                    if(traceLevels && traceLevels->server > 1)
                    {
                        Ice::Trace out(traceLevels->logger, traceLevels->serverCat);
                        out << "couldn't load `" << entry->getId() << "' on node `" << name << "':\n" << ex.reason;
                    }

                    ostringstream os;
                    os << "couldn't load `" << entry->getId() << "' on node `" << name << "':\n" << ex.reason;
                    entry->exception(make_exception_ptr(DeploymentException(os.str())));
                }
                catch(const Ice::Exception& ex)
                {
                    if(traceLevels && traceLevels->server > 1)
                    {
                        Ice::Trace out(traceLevels->logger, traceLevels->serverCat);
                        out << "couldn't load `" << entry->getId() << "' on node `" << name << "':\n" << ex;
                    }

                    entry->exception(make_exception_ptr(NodeUnreachableException(name, ex.what())));
                }
            };

        if(noRestart)
        {
            node->loadServerWithoutRestartAsync(desc, _cache.getReplicaName(), move(response), move(exception));
        }
        else
        {
            node->loadServerAsync(desc, _cache.getReplicaName(), move(response), move(exception));
        }
    }
    catch(const NodeUnreachableException&)
    {
        entry->exception(current_exception());
    }
}

void
NodeEntry::destroyServer(const shared_ptr<ServerEntry>& entry, const ServerInfo& info, chrono::seconds timeout, bool noRestart)
{
    try
    {
        shared_ptr<NodePrx> node;
        {
            unique_lock lock(_mutex);
            checkSession(lock);
            node = _session->getNode();

            //
            // Check if we should use a specific timeout (the load
            // call can deactivate the server and it can take some
            // time to deactivate, up to "deactivation-timeout"
            // seconds).
            //
            if(timeout > 0s)
            {
                int timeoutInMilliseconds = secondsToInt(timeout) * 1000;
                node = Ice::uncheckedCast<NodePrx>(node->ice_invocationTimeout(move(timeoutInMilliseconds)));
            }
        }

        if(_cache.getTraceLevels() && _cache.getTraceLevels()->server > 2)
        {
            Ice::Trace out(_cache.getTraceLevels()->logger, _cache.getTraceLevels()->serverCat);
            out << "unloading `" << info.descriptor->id << "' on node `" << _name << "'";
        }

        auto response = [traceLevels = _cache.getTraceLevels(), entry, name = _name]
            {
                if(traceLevels && traceLevels->server > 1)
                {
                    Ice::Trace out(traceLevels->logger, traceLevels->serverCat);
                    out << "unloaded `" << entry->getId() << "' on node `" << name << "'";
                }
                entry->destroyCallback();
            };

        auto exception = [traceLevels = _cache.getTraceLevels(), entry, name = _name](auto exptr)
            {
                try
                {
                    rethrow_exception(exptr);
                }
                catch(const DeploymentException& ex)
                {
                    if(traceLevels && traceLevels->server > 1)
                    {
                        Ice::Trace out(traceLevels->logger, traceLevels->serverCat);
                        out << "couldn't unload `" << entry->getId() << "' on node `" << name << "':\n" << ex.reason;
                    }

                    ostringstream os;
                    os << "couldn't unload `" << entry->getId() << "' on node `" << name << "':\n" << ex.reason;
                    entry->exception(make_exception_ptr(DeploymentException(os.str())));
                }
                catch(const Ice::Exception& ex)
                {
                    if(traceLevels && traceLevels->server > 1)
                    {
                        Ice::Trace out(traceLevels->logger, traceLevels->serverCat);
                        out << "couldn't unload `" << entry->getId() << "' on node `" << name << "':\n" << ex;
                    }
                    entry->exception(make_exception_ptr(NodeUnreachableException(name, ex.what())));
                }
            };

        if(noRestart)
        {
            node->destroyServerWithoutRestartAsync(info.descriptor->id, info.uuid, info.revision,
                                                   _cache.getReplicaName(), move(response), move(exception));
        }
        else
        {
            node->destroyServerAsync(info.descriptor->id, info.uuid, info.revision, _cache.getReplicaName(),
                                     move(response), move(exception));
        }
    }
    catch(const NodeUnreachableException&)
    {
        entry->exception(current_exception());
    }
}

ServerInfo
NodeEntry::getServerInfo(const ServerInfo& server, const shared_ptr<SessionI>& session)
{
    unique_lock lock(_mutex);
    checkSession(lock);

    ServerInfo info = server;
    info.descriptor = getServerDescriptor(server, session);
    assert(info.descriptor);
    return info;
}

shared_ptr<InternalServerDescriptor>
NodeEntry::getInternalServerDescriptor(const ServerInfo& server, const shared_ptr<SessionI>& session)
{
    unique_lock lock(_mutex);
    checkSession(lock);

    ServerInfo info = server;
    try
    {
        info.descriptor = getServerDescriptor(server, session);
    }
    catch(const DeploymentException&)
    {
        //
        // We ignore the deployment error for now (which can
        // only be caused in theory by session variables not
        // being defined because the server isn't
        // allocated...)
        //
    }
    return getInternalServerDescriptor(info);
}

void
NodeEntry::checkSession(unique_lock<mutex>& lock) const
{
    if(_session)
    {
        if(_session->isDestroyed())
        {
            throw NodeUnreachableException(_name, "the node is not active");
        }
        return;
    }
    else if(!_proxy && !_registering)
    {
        throw NodeUnreachableException(_name, "the node is not active");
    }
    else if(_proxy)
    {
        //
        // If the node proxy is set, we attempt to get the node to
        // register with this registry.
        //
        assert(!_registering);

        if(_cache.getTraceLevels() && _cache.getTraceLevels()->node > 0)
        {
            Ice::Trace out(_cache.getTraceLevels()->logger, _cache.getTraceLevels()->nodeCat);
            out << "creating node `" << _name << "' session";
        }

        //
        // NOTE: setting _registering to true must be done before the
        // call otherwise if the callback is call immediately we'll
        // hang in the while loop.
        //
        _registering = true;

        // 'this' is only ever accessed though the self removing pointer, ensuring _selfRemovingPtr is always
        // valid and its access is thread safe
        auto self = _selfRemovingPtr.lock();
        assert(self);
        _proxy->registerWithReplicaAsync(_cache.getReplicaCache().getInternalRegistry(),
                                        [self]
                                        {
                                            self->finishedRegistration();
                                        },
                                        [self] (exception_ptr ex)
                                        {
                                            self->finishedRegistration(ex);
                                        });
        _proxy = nullptr; // Registration with the proxy is only attempted once.

    }

    // Consider the node down if it doesn't respond promptly.
     _condVar.wait_for(lock, 10s, [this] { return !_registering; });

    if(!_session || _session->isDestroyed())
    {
        throw NodeUnreachableException(_name, "the node is not active");
    }
}

void
NodeEntry::setProxy(const shared_ptr<NodePrx>& node)
{
    lock_guard lock(_mutex);

    //
    // If the node has already established a session with the
    // registry, no need to remember its proxy, we don't need to get
    // it to register with this registry since it's already
    // registered.
    //
    if(!_session)
    {
        _proxy = node;
    }
}

void
NodeEntry::finishedRegistration()
{
    lock_guard lock(_mutex);
    if(_cache.getTraceLevels() && _cache.getTraceLevels()->node > 0)
    {
        Ice::Trace out(_cache.getTraceLevels()->logger, _cache.getTraceLevels()->nodeCat);
        if(_session)
        {
            out << "node `" << _name << "' session created";
        }
        else
        {
            out << "node `" << _name << "' session creation failed";
        }
    }

    if(_registering)
    {
        _registering = false;
        _condVar.notify_all();
    }
}

void
NodeEntry::finishedRegistration(exception_ptr exptr)
{
    lock_guard lock(_mutex);
    if(_cache.getTraceLevels() && _cache.getTraceLevels()->node > 0)
    {
        try
        {
            rethrow_exception(exptr);
        }
        catch(const std::exception& ex)
        {
            Ice::Trace out(_cache.getTraceLevels()->logger, _cache.getTraceLevels()->nodeCat);
            out << "node `" << _name << "' session creation failed:\n" << ex.what();
        }
    }

    if(_registering)
    {
        _registering = false;
        _condVar.notify_all();
    }
}

shared_ptr<ServerDescriptor>
NodeEntry::getServerDescriptor(const ServerInfo& server, const shared_ptr<SessionI>& session)
{
    assert(_session);

    Resolver resolve(_session->getInfo(), _cache.getCommunicator());
    resolve.setReserved("application", server.application);
    resolve.setReserved("server", server.descriptor->id);
    resolve.setContext("server `${server}'");

    if(session)
    {
        resolve.setReserved("session.id", session->getId());
    }

    auto iceBox = dynamic_pointer_cast<IceBoxDescriptor>(server.descriptor);
    if(iceBox)
    {
        return IceBoxHelper(iceBox).instantiate(resolve, PropertyDescriptorSeq(), PropertySetDescriptorDict());
    }
    else
    {
        return ServerHelper(server.descriptor).instantiate(resolve, PropertyDescriptorSeq(),
                                                           PropertySetDescriptorDict());
    }
}

shared_ptr<InternalServerDescriptor>
NodeEntry::getInternalServerDescriptor(const ServerInfo& info) const
{
    //
    // Note that at this point all variables in info have been resolved
    //
    assert(_session);

    shared_ptr<InternalServerDescriptor> server = make_shared<InternalServerDescriptor>();
    server->id = info.descriptor->id;
    server->application = info.application;
    server->uuid = info.uuid;
    server->revision = info.revision;
    server->sessionId = info.sessionId;
    server->exe = info.descriptor->exe;
    server->pwd = info.descriptor->pwd;
    server->user = info.descriptor->user;
    server->activation = info.descriptor->activation;
    server->activationTimeout = info.descriptor->activationTimeout;
    server->deactivationTimeout = info.descriptor->deactivationTimeout;
    server->services = Ice::StringSeq();
    server->options = info.descriptor->options;
    server->envs = info.descriptor->envs;

    // server->logs: assigned for each communicator (see below)
    // server->adapters: assigned for each communicator (see below)
    // server->properties: assigned for each communicator (see below)

    //
    // Add server properties.
    //
    PropertyDescriptorSeq& props = server->properties["config"];
    props.push_back(createProperty("# Server configuration"));

    //
    // For newer versions of Ice, we generate Ice.Admin properties:
    //
    int iceVersion = 0;
    if(info.descriptor->iceVersion != "")
    {
        iceVersion = getMMVersion(info.descriptor->iceVersion);
    }

    server->processRegistered = false;
    if(iceVersion == 0 || iceVersion >= 30300)
    {
        props.push_back(createProperty("Ice.Admin.ServerId", info.descriptor->id));

        if(hasProperty(info.descriptor->propertySet.properties, "Ice.Admin.Enabled"))
        {
            // Ice.Admin.Enabled explicitely set, leave Ice.Admin.Endpoints alone
            server->processRegistered =
                getPropertyAsInt(info.descriptor->propertySet.properties, "Ice.Admin.Enabled") > 0;
        }
        else if(hasProperty(info.descriptor->propertySet.properties, "Ice.Admin.Endpoints"))
        {
            // Ice.Admin.Endpoints explicitely set, check if not ""
            server->processRegistered =
                getProperty(info.descriptor->propertySet.properties, "Ice.Admin.Endpoints") != "";
        }
        else
        {
            props.push_back(createProperty("Ice.Admin.Endpoints", "tcp -h 127.0.0.1"));
            props.push_back(createProperty("Ice.Admin.ServerName", "127.0.0.1"));
            server->processRegistered = true;
        }
    }
    else
    {
        props.push_back(createProperty("Ice.ServerId", info.descriptor->id));
        //
        // Prior to Ice 3.3, use adapter's registerProcess to compute server->processRegistered;
        // see ToInternalServerDescriptor::operator() above
        //
    }

    props.push_back(createProperty("Ice.ProgramName", info.descriptor->id));

    //
    // Add IceBox properties.
    //
    string servicesStr;
    auto iceBox = dynamic_pointer_cast<IceBoxDescriptor>(info.descriptor);
    if(iceBox)
    {
        for(const auto& serviceInstance : iceBox->services)
        {
            const auto& s = serviceInstance.descriptor;
            const string path = _session->getInfo()->dataDir + "/servers/" + server->id + "/config/config_" + s->name;

            //
            // We escape the path here because the command-line option --Ice.Config=xxx will be parsed an encoded
            // (escaped) property
            // For example, \\server\dir\file.cfg needs to become \\\server\dir\file.cfg or \\\\server\\dir\\file.cfg.
            //
            props.push_back(createProperty("IceBox.Service." + s->name, s->entry + " --Ice.Config='"
                                           + escapeProperty(path) + "'"));

            if(servicesStr.empty())
            {
                servicesStr = s->name;
            }
            else
            {
                servicesStr += " " + s->name;
            }
        }

        props.push_back(createProperty("IceBox.LoadOrder", servicesStr));
    }

    //
    // Now, for each communicator of the descriptor, add the necessary
    // logs, adapters and properties to the internal server
    // descriptor.
    //
    forEachCommunicator(info.descriptor,
        [server, node = _session->getInfo(), iceVersion](const auto& desc)
        {
            //
            // Figure out the configuration file name for the communicator
            // (if it's a service, it's "config_<service name>", if it's
            // the server, it's just "config").
            //
            string filename = "config";
            auto svc = dynamic_pointer_cast<ServiceDescriptor>(desc);
            if(svc)
            {
                filename += "_" + svc->name;
                server->services->push_back(svc->name);
            }

            PropertyDescriptorSeq& serverProps = server->properties[filename];
            PropertyDescriptorSeq communicatorProps = desc->propertySet.properties;

            //
            // If this is a service communicator and the IceBox server has Admin
            // enabled or Admin endpoints configured, we ignore the server-lifetime attributes
            // of the service object adapters and assume it's set to false.
            //
            bool ignoreServerLifetime = false;
            if(svc)
            {
                if(iceVersion == 0 || iceVersion >= 30300)
                {
                    if(getPropertyAsInt(server->properties["config"], "Ice.Admin.Enabled") > 0 ||
                    getProperty(server->properties["config"], "Ice.Admin.Endpoints") != "")
                    {
                        ignoreServerLifetime = true;
                    }
                }
            }
            //
            // Add the adapters and their configuration.
            //
            for(const auto& adapter : desc->adapters)
            {
                server->adapters.push_back(make_shared<InternalAdapterDescriptor>(adapter.id,
                                                                                ignoreServerLifetime ? false :
                                                                                adapter.serverLifetime));

                serverProps.push_back(createProperty("# Object adapter " + adapter.name));

                PropertyDescriptor prop = removeProperty(communicatorProps, adapter.name + ".Endpoints");
                prop.name = adapter.name + ".Endpoints";
                serverProps.push_back(prop);
                serverProps.push_back(createProperty(adapter.name + ".AdapterId", adapter.id));

                if(!adapter.replicaGroupId.empty())
                {
                    serverProps.push_back(createProperty(adapter.name + ".ReplicaGroupId", adapter.replicaGroupId));
                }

                //
                // Ignore the register process attribute if the server is using Ice >= 3.3.0
                //
                if(iceVersion != 0 && iceVersion < 30300)
                {
                    if(adapter.registerProcess)
                    {
                        serverProps.push_back(createProperty(adapter.name + ".RegisterProcess", "1"));
                        server->processRegistered = true;
                    }
                }
            }

            server->logs.insert(server->logs.end(), desc->logs.begin(), desc->logs.end());

            //
            // Copy the communicator descriptor properties.
            //
            if(!communicatorProps.empty())
            {
                if(svc)
                {
                    serverProps.push_back(createProperty("# Service descriptor properties"));
                }
                else
                {
                    serverProps.push_back(createProperty("# Server descriptor properties"));
                }
                copy(communicatorProps.begin(), communicatorProps.end(), back_inserter(serverProps));
            }

            //
            // For Ice servers > 3.3.0 escape the properties.
            //
            if(iceVersion == 0 || iceVersion >= 30300)
            {
                for(PropertyDescriptorSeq::iterator p = serverProps.begin(); p != serverProps.end(); ++p)
                {
                    if(p->name.find('#') != 0 || !p->value.empty())
                    {
                        p->name = escapeProperty(p->name, true);
                        p->value = escapeProperty(p->value);
                    }
                }
            }
        });
    return server;
}
