//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceUtil/Functional.h>
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

namespace IceGrid
{

struct ToInternalServerDescriptor : std::unary_function<CommunicatorDescriptorPtr&, void>
{
    ToInternalServerDescriptor(const InternalServerDescriptorPtr& descriptor, const InternalNodeInfoPtr& node,
                               int iceVersion) :
        _desc(descriptor),
        _node(node),
        _iceVersion(iceVersion)
    {
    }

    void
    operator()(const CommunicatorDescriptorPtr& desc)
    {
        //
        // Figure out the configuration file name for the communicator
        // (if it's a service, it's "config_<service name>", if it's
        // the server, it's just "config").
        //
        string filename = "config";
        ServiceDescriptorPtr svc = ServiceDescriptorPtr::dynamicCast(desc);
        if(svc)
        {
            filename += "_" + svc->name;
            _desc->services->push_back(svc->name);
        }

        PropertyDescriptorSeq& props = _desc->properties[filename];
        PropertyDescriptorSeq communicatorProps = desc->propertySet.properties;

        //
        // If this is a service communicator and the IceBox server has Admin
        // enabled or Admin endpoints configured, we ignore the server-lifetime attributes
        // of the service object adapters and assume it's set to false.
        //
        bool ignoreServerLifetime = false;
        if(svc)
        {
            if(_iceVersion == 0 || _iceVersion >= 30300)
            {
                if(getPropertyAsInt(_desc->properties["config"], "Ice.Admin.Enabled") > 0 ||
                   getProperty(_desc->properties["config"], "Ice.Admin.Endpoints") != "")
                {
                    ignoreServerLifetime = true;
                }
            }
        }
        //
        // Add the adapters and their configuration.
        //
        for(AdapterDescriptorSeq::const_iterator q = desc->adapters.begin(); q != desc->adapters.end(); ++q)
        {
            _desc->adapters.push_back(new InternalAdapterDescriptor(q->id,
                                                                    ignoreServerLifetime ? false : q->serverLifetime));

            props.push_back(createProperty("# Object adapter " + q->name));
            PropertyDescriptor prop = removeProperty(communicatorProps, q->name + ".Endpoints");
            prop.name = q->name + ".Endpoints";
            props.push_back(prop);
            props.push_back(createProperty(q->name + ".AdapterId", q->id));
            if(!q->replicaGroupId.empty())
            {
                props.push_back(createProperty(q->name + ".ReplicaGroupId", q->replicaGroupId));
            }

            //
            // Ignore the register process attribute if the server is using Ice >= 3.3.0
            //
            if(_iceVersion != 0 && _iceVersion < 30300)
            {
                if(q->registerProcess)
                {
                    props.push_back(createProperty(q->name + ".RegisterProcess", "1"));
                    _desc->processRegistered = true;
                }
            }
        }

        _desc->logs.insert(_desc->logs.end(), desc->logs.begin(), desc->logs.end());

        //
        // Copy the communicator descriptor properties.
        //
        if(!communicatorProps.empty())
        {
            if(svc)
            {
                props.push_back(createProperty("# Service descriptor properties"));
            }
            else
            {
                props.push_back(createProperty("# Server descriptor properties"));
            }
            copy(communicatorProps.begin(), communicatorProps.end(), back_inserter(props));
        }

        //
        // For Ice servers > 3.3.0 escape the properties.
        //
        if(_iceVersion == 0 || _iceVersion >= 30300)
        {
            for(PropertyDescriptorSeq::iterator p = props.begin(); p != props.end(); ++p)
            {
                if(p->name.find('#') != 0 || !p->value.empty())
                {
                    p->name = escapeProperty(p->name, true);
                    p->value = escapeProperty(p->value);
                }
            }
        }
    }

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
        PropertyDescriptor desc;
        desc.name = name;
        desc.value = value;
        return desc;
    }

    InternalServerDescriptorPtr _desc;
    InternalNodeInfoPtr _node;
    int _iceVersion;
};

class LoadCB : public virtual IceUtil::Shared
{
public:

    LoadCB(const TraceLevelsPtr& traceLevels, const ServerEntryPtr& server, const string& node, int timeout) :
        _traceLevels(traceLevels), _server(server), _id(server->getId()), _node(node), _timeout(timeout)
    {
    }

    void
    response(const ServerPrx& server, const AdapterPrxDict& adapters, int at, int dt)
    {
        if(_traceLevels && _traceLevels->server > 1)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->serverCat);
            out << "loaded `" << _id << "' on node `" << _node << "'";
        }

        //
        // Add the node session timeout on the proxies to ensure the
        // timeout is large enough.
        //
        _server->loadCallback(server, adapters, at + _timeout, dt + _timeout);
    }

    void
    exception(const Ice::Exception& lex)
    {
        try
        {
            lex.ice_throw();
        }
        catch(const DeploymentException& ex)
        {
            if(_traceLevels && _traceLevels->server > 1)
            {
                Ice::Trace out(_traceLevels->logger, _traceLevels->serverCat);
                out << "couldn't load `" << _id << "' on node `" << _node << "':\n" << ex.reason;
            }

            ostringstream os;
            os << "couldn't load `" << _id << "' on node `" << _node << "':\n" << ex.reason;
            _server->exception(DeploymentException(os.str()));
        }
        catch(const Ice::Exception& ex)
        {
            if(_traceLevels && _traceLevels->server > 1)
            {
                Ice::Trace out(_traceLevels->logger, _traceLevels->serverCat);
                out << "couldn't load `" << _id << "' on node `" << _node << "':\n" << ex;
            }

            ostringstream os;
            os << ex;
            _server->exception(NodeUnreachableException(_node, os.str()));
        }
    }

private:

    const TraceLevelsPtr _traceLevels;
    const ServerEntryPtr _server;
    const string _id;
    const string _node;
    const int _timeout;
};

class DestroyCB : public virtual IceUtil::Shared
{
public:

    DestroyCB(const TraceLevelsPtr& traceLevels, const ServerEntryPtr& server, const string& node) :
        _traceLevels(traceLevels), _server(server), _id(server->getId()), _node(node)
    {
    }

    void
    response()
    {
        if(_traceLevels && _traceLevels->server > 1)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->serverCat);
            out << "unloaded `" << _id << "' on node `" << _node << "'";
        }
        _server->destroyCallback();
    }

    void
    exception(const Ice::Exception& dex)
    {
        try
        {
            dex.ice_throw();
        }
        catch(const DeploymentException& ex)
        {
            if(_traceLevels && _traceLevels->server > 1)
            {
                Ice::Trace out(_traceLevels->logger, _traceLevels->serverCat);
                out << "couldn't unload `" << _id << "' on node `" << _node << "':\n" << ex.reason;
            }

            ostringstream os;
            os << "couldn't unload `" << _id << "' on node `" << _node << "':\n" << ex.reason;
            _server->exception(DeploymentException(os.str()));
        }
        catch(const Ice::Exception& ex)
        {
            if(_traceLevels && _traceLevels->server > 1)
            {
                Ice::Trace out(_traceLevels->logger, _traceLevels->serverCat);
                out << "couldn't unload `" << _id << "' on node `" << _node << "':\n" << ex;
            }
            ostringstream os;
            os << ex;
            _server->exception(NodeUnreachableException(_node, os.str()));
        }
    }

private:

    const TraceLevelsPtr _traceLevels;
    const ServerEntryPtr _server;
    const string _id;
    const string _node;
};

}

NodeCache::NodeCache(const Ice::CommunicatorPtr& communicator, ReplicaCache& replicaCache, const string& replicaName) :
    _communicator(communicator),
    _replicaName(replicaName),
    _replicaCache(replicaCache)
{
}

NodeEntryPtr
NodeCache::get(const string& name, bool create) const
{
    Lock sync(*this);
    NodeEntryPtr entry = getImpl(name);
    if(!entry && create)
    {
        NodeCache& self = const_cast<NodeCache&>(*this);
        entry = new NodeEntry(self, name);
        self.addImpl(name, entry);
    }
    if(!entry)
    {
        throw NodeNotExistException(name);
    }
    return entry;
}

NodeEntry::NodeEntry(NodeCache& cache, const std::string& name) :
    _cache(cache),
    _ref(0),
    _name(name),
    _registering(false)
{
}

NodeEntry::~NodeEntry()
{
}

void
NodeEntry::addDescriptor(const string& application, const NodeDescriptor& descriptor)
{
    Lock sync(*this);
    _descriptors.insert(make_pair(application, descriptor));
}

void
NodeEntry::removeDescriptor(const string& application)
{
    Lock sync(*this);
    _descriptors.erase(application);
}

void
NodeEntry::addServer(const ServerEntryPtr& entry)
{
    Lock sync(*this);
    _servers.insert(make_pair(entry->getId(), entry));
}

void
NodeEntry::removeServer(const ServerEntryPtr& entry)
{
    Lock sync(*this);
    _servers.erase(entry->getId());
}

void
NodeEntry::setSession(const NodeSessionIPtr& session)
{
    Lock sync(*this);

    if(session)
    {
        while(_session)
        {
            if(_session->isDestroyed())
            {
                // If the current session has just been destroyed, wait for the setSession(0) call.
                assert(session != _session);
                wait();
            }
            else
            {
                NodeSessionIPtr s = _session;
                sync.release();
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
                sync.acquire();
            }
        }

        //
        // Clear the saved proxy, the node has established a session
        // so we won't need anymore to try to register it with this
        // registry.
        //
        _proxy = 0;
    }
    else
    {
        if(!_session)
        {
            return;
        }
    }

    _session = session;
    notifyAll();

    if(_registering)
    {
        _registering = false;
        notifyAll();
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

NodePrx
NodeEntry::getProxy() const
{
    Lock sync(*this);
    checkSession();
    return _session->getNode();
}

InternalNodeInfoPtr
NodeEntry::getInfo() const
{
    Lock sync(*this);
    checkSession();
    return _session->getInfo();
}

ServerEntrySeq
NodeEntry::getServers() const
{
    Lock sync(*this);
    ServerEntrySeq entries;
    for(map<string, ServerEntryPtr>::const_iterator p = _servers.begin(); p != _servers.end(); ++p)
    {
        entries.push_back(p->second);
    }
    return entries;
}

LoadInfo
NodeEntry::getLoadInfoAndLoadFactor(const string& application, float& loadFactor) const
{
    Lock sync(*this);
    checkSession();

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

NodeSessionIPtr
NodeEntry::getSession() const
{
    Lock sync(*this);
    checkSession();
    return _session;
}

Ice::ObjectPrx
NodeEntry::getAdminProxy() const
{
    Ice::ObjectPrx prx = getProxy();
    assert(prx);
    Ice::Identity adminId;
    adminId.name = "NodeAdmin-" + _name ;
    adminId.category = prx->ice_getIdentity().category;
    return prx->ice_identity(adminId);
}

bool
NodeEntry::canRemove()
{
    Lock sync(*this);
    return _servers.empty() && !_session && _descriptors.empty();
}

void
NodeEntry::loadServer(const ServerEntryPtr& entry, const ServerInfo& server, const SessionIPtr& session, int timeout,
                      bool noRestart)
{
    try
    {
        NodePrx node;
        int sessionTimeout;
        InternalServerDescriptorPtr desc;
        {
            Lock sync(*this);
            checkSession();
            node = _session->getNode();
            sessionTimeout = _session->getTimeout(Ice::emptyCurrent);

            //
            // Check if we should use a specific timeout (the load
            // call can deactivate the server and it can take some
            // time to deactivate, up to "deactivation-timeout"
            // seconds).
            //
            if(timeout > 0)
            {
                node = NodePrx::uncheckedCast(node->ice_invocationTimeout(timeout * 1000));
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

        if(noRestart)
        {
            node->begin_loadServerWithoutRestart(desc, _cache.getReplicaName(),
                                                 newCallback_Node_loadServerWithoutRestart(
                                                     new LoadCB(_cache.getTraceLevels(), entry, _name, sessionTimeout),
                                                     &LoadCB::response,
                                                     &LoadCB::exception));
        }
        else
        {
            node->begin_loadServer(desc, _cache.getReplicaName(),
                                   newCallback_Node_loadServer(
                                       new LoadCB(_cache.getTraceLevels(), entry, _name, sessionTimeout),
                                       &LoadCB::response,
                                       &LoadCB::exception));
        }
    }
    catch(const NodeUnreachableException& ex)
    {
        entry->exception(ex);
    }
}

void
NodeEntry::destroyServer(const ServerEntryPtr& entry, const ServerInfo& info, int timeout, bool noRestart)
{
    try
    {
        NodePrx node;
        {
            Lock sync(*this);
            checkSession();
            node = _session->getNode();

            //
            // Check if we should use a specific timeout (the load
            // call can deactivate the server and it can take some
            // time to deactivate, up to "deactivation-timeout"
            // seconds).
            //
            if(timeout > 0)
            {
                node = NodePrx::uncheckedCast(node->ice_invocationTimeout(timeout * 1000));
            }
        }

        if(_cache.getTraceLevels() && _cache.getTraceLevels()->server > 2)
        {
            Ice::Trace out(_cache.getTraceLevels()->logger, _cache.getTraceLevels()->serverCat);
            out << "unloading `" << info.descriptor->id << "' on node `" << _name << "'";
        }

        if(noRestart)
        {
            node->begin_destroyServerWithoutRestart(info.descriptor->id, info.uuid, info.revision,
                                                    _cache.getReplicaName(),
                                                    newCallback_Node_destroyServerWithoutRestart(
                                                        new DestroyCB(_cache.getTraceLevels(), entry, _name),
                                                        &DestroyCB::response,
                                                        &DestroyCB::exception));
        }
        else
        {
            node->begin_destroyServer(info.descriptor->id, info.uuid, info.revision, _cache.getReplicaName(),
                                      newCallback_Node_destroyServer(
                                          new DestroyCB(_cache.getTraceLevels(), entry, _name),
                                          &DestroyCB::response,
                                          &DestroyCB::exception));
        }
    }
    catch(const NodeUnreachableException& ex)
    {
        entry->exception(ex);
    }
}

ServerInfo
NodeEntry::getServerInfo(const ServerInfo& server, const SessionIPtr& session)
{
    Lock sync(*this);
    checkSession();

    ServerInfo info = server;
    info.descriptor = getServerDescriptor(server, session);
    assert(info.descriptor);
    return info;
}

InternalServerDescriptorPtr
NodeEntry::getInternalServerDescriptor(const ServerInfo& server, const SessionIPtr& session)
{
    Lock sync(*this);
    checkSession();

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
NodeEntry::__incRef()
{
    Lock sync(*this);
    assert(_ref >= 0);
    ++_ref;
}

void
NodeEntry::__decRef()
{
    //
    // The node entry implements its own reference counting. If the
    // reference count drops to 1, this means that only the cache
    // holds a reference on the node entry. If that's the case, we
    // check if the node entry can be removed or not and if it can be
    // removed we remove it from the cache map.
    //

    bool doRemove = false;
    bool doDelete = false;
    {
        Lock sync(*this); // We use a recursive mutex so it's fine to
                          // create Ptr with the mutex locked.
        assert(_ref > 0);
        --_ref;

        if(_ref == 1)
        {
            doRemove = canRemove();
        }
        else if(_ref == 0)
        {
            doDelete = true;
        }
    }

    if(doRemove)
    {
        _cache.remove(_name);
    }
    else if(doDelete)
    {
        delete this;
    }
}

void
NodeEntry::checkSession() const
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
        NodeEntry* self = const_cast<NodeEntry*>(this);
        _proxy->begin_registerWithReplica(_cache.getReplicaCache().getInternalRegistry(),
                                          newCallback_Node_registerWithReplica(self,
                                                                               &NodeEntry::finishedRegistration,
                                                                               &NodeEntry::finishedRegistration));
        _proxy = 0; // Registration with the proxy is only attempted once.
    }

    while(_registering)
    {
        if(!timedWait(IceUtil::Time::seconds(10)))
        {
            break; // Consider the node down if it doesn't respond promptly.
        }
    }

    if(!_session || _session->isDestroyed())
    {
        throw NodeUnreachableException(_name, "the node is not active");
    }
}

void
NodeEntry::setProxy(const NodePrx& node)
{
    Lock sync(*this);

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
    Lock sync(*this);
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
        notifyAll();
    }
}

void
NodeEntry::finishedRegistration(const Ice::Exception& ex)
{
    Lock sync(*this);
    if(_cache.getTraceLevels() && _cache.getTraceLevels()->node > 0)
    {
        Ice::Trace out(_cache.getTraceLevels()->logger, _cache.getTraceLevels()->nodeCat);
        out << "node `" << _name << "' session creation failed:\n" << ex;
    }

    if(_registering)
    {
        _registering = false;
        notifyAll();
    }
}

ServerDescriptorPtr
NodeEntry::getServerDescriptor(const ServerInfo& server, const SessionIPtr& session)
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

    IceBoxDescriptorPtr iceBox = IceBoxDescriptorPtr::dynamicCast(server.descriptor);
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

InternalServerDescriptorPtr
NodeEntry::getInternalServerDescriptor(const ServerInfo& info) const
{
    //
    // Note that at this point all variables in info have been resolved
    //
    assert(_session);

    InternalServerDescriptorPtr server = new InternalServerDescriptor();
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
            props.push_back(createProperty("Ice.Admin.Endpoints", "tcp -h localhost"));
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
    IceBoxDescriptorPtr iceBox = IceBoxDescriptorPtr::dynamicCast(info.descriptor);
    if(iceBox)
    {
        for(ServiceInstanceDescriptorSeq::const_iterator p = iceBox->services.begin(); p != iceBox->services.end();++p)
        {
            ServiceDescriptorPtr s = p->descriptor;
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

        if(iceVersion != 0 && iceVersion < 30300)
        {
            if(hasProperty(iceBox->propertySet.properties, "IceBox.ServiceManager.RegisterProcess"))
            {
                if(getProperty(iceBox->propertySet.properties, "IceBox.ServiceManager.RegisterProcess") != "0")
                {
                    server->processRegistered = true;
                }
            }
            else
            {
                props.push_back(createProperty("IceBox.ServiceManager.RegisterProcess", "1"));
                server->processRegistered = true;
            }
            if(!hasProperty(iceBox->propertySet.properties, "IceBox.ServiceManager.Endpoints"))
            {
                props.push_back(createProperty("IceBox.ServiceManager.Endpoints", "tcp -h 127.0.0.1"));
            }
        }
        if(!hasProperty(info.descriptor->propertySet.properties, "IceBox.InstanceName") &&
            hasProperty(iceBox->propertySet.properties, "IceBox.ServiceManager.Endpoints"))
        {
            props.push_back(createProperty("IceBox.InstanceName", server->id));
        }
    }

    //
    // Now, for each communicator of the descriptor, add the necessary
    // logs, adapters and properties to the internal server
    // descriptor.
    //
    forEachCommunicator(ToInternalServerDescriptor(server, _session->getInfo(), iceVersion))(info.descriptor);
    return server;
}
