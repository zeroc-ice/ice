//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/UUID.h>

#include <Ice/Ice.h>
#include <Ice/LoggerUtil.h>
#include <Ice/TraceUtil.h>

#include <IceGrid/AdminI.h>
#include <IceGrid/RegistryI.h>
#include <IceGrid/Database.h>
#include <IceGrid/Util.h>
#include <IceGrid/DescriptorParser.h>
#include <IceGrid/DescriptorHelper.h>
#include <IceGrid/AdminSessionI.h>
#include <IceGrid/NodeSessionI.h>

#include "SynchronizationException.h"

using namespace std;
using namespace std::chrono;
using namespace Ice;
using namespace IceGrid;

namespace
{

    class ServerProxyWrapper
    {
    public:
        ServerProxyWrapper(const shared_ptr<Database>& database, const string& id) : _id(id)
        {
            try
            {
                _proxy = database->getServer(_id)->getProxy(_activationTimeout, _deactivationTimeout, _node, false, 5s);
            }
            catch (const SynchronizationException&)
            {
                throw DeploymentException("server is being updated");
            }
        }

        ServerProxyWrapper(const ServerProxyWrapper& wrapper) = default;

        template <typename Func, typename... Args> auto invoke(Func&& f, Args&&... args)
        {
            try
            {
                return std::invoke(std::forward<Func>(f), _proxy, std::forward<Args>(args)...,
                                   ::Ice::noExplicitContext);
            }
            catch (const Ice::Exception&)
            {
                handleException(current_exception());
                throw; // keep compiler happy
            }
        }

        template <typename Func>
        auto invokeAsync(Func&& f, function<void()> response, function<void(exception_ptr)> exception)
        {
            auto exceptionWrapper = [this, exception = std::move(exception)](exception_ptr ex)
            {
                try
                {
                    handleException(ex);
                }
                catch (const std::exception&)
                {
                    exception(current_exception());
                }
            };
            return std::invoke(std::forward<Func>(f), _proxy, std::move(response), std::move(exceptionWrapper), nullptr,
                               Ice::noExplicitContext);
        }

        void useActivationTimeout()
        {
            auto timeout = secondsToInt(_activationTimeout) * 1000;
            _proxy = _proxy->ice_invocationTimeout(timeout);
        }

        void useDeactivationTimeout()
        {
            auto timeout = secondsToInt(_deactivationTimeout) * 1000;
            _proxy = _proxy->ice_invocationTimeout(timeout);
        }

        ServerPrx* operator->()
        {
            if (_proxy)
            {
                return &_proxy.value();
            }
            else
            {
                return nullptr;
            }
        }

        void handleException(exception_ptr ex) const
        {
            try
            {
                rethrow_exception(ex);
            }
            catch (const Ice::UserException&)
            {
                throw;
            }
            catch (const Ice::ObjectNotExistException&)
            {
                throw ServerNotExistException(_id);
            }
            catch (const Ice::LocalException& e)
            {
                throw NodeUnreachableException(_node, e.what());
            }
        }

    private:
        string _id;
        ServerPrxPtr _proxy;
        chrono::seconds _activationTimeout;
        chrono::seconds _deactivationTimeout;
        string _node;
    };

    class AMDPatcherFeedbackAggregator : public PatcherFeedbackAggregator
    {
    public:
        AMDPatcherFeedbackAggregator(function<void()> response,
                                     function<void(exception_ptr)> exception,
                                     Ice::Identity id,
                                     const shared_ptr<TraceLevels>& traceLevels,
                                     const string& type,
                                     const string& name,
                                     int nodeCount)
            : PatcherFeedbackAggregator(id, traceLevels, type, name, nodeCount),
              _response(std::move(response)),
              _exception(std::move(exception))
        {
        }

    private:
        void response() { _response(); }

        void exception(exception_ptr ex) { _exception(ex); }

        function<void()> _response;
        function<void(exception_ptr)> _exception;
    };

    shared_ptr<AMDPatcherFeedbackAggregator> static newPatcherFeedback(function<void()> response,
                                                                       function<void(exception_ptr)> exception,
                                                                       Ice::Identity id,
                                                                       const shared_ptr<TraceLevels>& traceLevels,
                                                                       const string& type,
                                                                       const string& name,
                                                                       int nodeCount)
    {
        return make_shared<AMDPatcherFeedbackAggregator>(response, exception, id, traceLevels, type, name, nodeCount);
    }

}

AdminI::AdminI(const shared_ptr<Database>& database,
               const shared_ptr<RegistryI>& registry,
               const shared_ptr<AdminSessionI>& session)
    : _database(database),
      _registry(registry),
      _traceLevels(_database->getTraceLevels()),
      _session(session)
{
}

void
AdminI::addApplication(ApplicationDescriptor descriptor, const Current&)
{
    checkIsReadOnly();

    ApplicationInfo info;
    info.createTime = info.updateTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    info.createUser = info.updateUser = _session->getId();
    info.descriptor = descriptor;
    info.revision = 1;
    info.uuid = Ice::generateUUID();

    _database->addApplication(info, _session.get());
}

void
AdminI::syncApplication(ApplicationDescriptor descriptor, const Current&)
{
    checkIsReadOnly();
    _database->syncApplicationDescriptor(descriptor, false, _session.get());
}

void
AdminI::updateApplication(ApplicationUpdateDescriptor descriptor, const Current&)
{
    checkIsReadOnly();

    ApplicationUpdateInfo update;
    update.updateTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    update.updateUser = _session->getId();
    update.descriptor = descriptor;
    update.revision = -1; // The database will set it.
    _database->updateApplication(update, false, _session.get());
}

void
AdminI::syncApplicationWithoutRestart(ApplicationDescriptor descriptor, const Current&)
{
    checkIsReadOnly();
    _database->syncApplicationDescriptor(descriptor, true, _session.get());
}

void
AdminI::updateApplicationWithoutRestart(ApplicationUpdateDescriptor descriptor, const Current&)
{
    checkIsReadOnly();

    ApplicationUpdateInfo update;
    update.updateTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    update.updateUser = _session->getId();
    update.descriptor = descriptor;
    update.revision = -1; // The database will set it.
    _database->updateApplication(update, true, _session.get());
}

void
AdminI::removeApplication(string name, const Current&)
{
    checkIsReadOnly();
    _database->removeApplication(std::move(name), _session.get());
}

void
AdminI::instantiateServer(string app, string node, ServerInstanceDescriptor desc, const Current&)
{
    checkIsReadOnly();
    _database->instantiateServer(std::move(app), std::move(node), std::move(desc), _session.get());
}

void
AdminI::patchApplicationAsync(string name,
                              bool shutdown,
                              function<void()> response,
                              function<void(exception_ptr)> exception,
                              const Current& current)
{
    ApplicationHelper helper(current.adapter->getCommunicator(), _database->getApplicationInfo(name).descriptor);
    DistributionDescriptor appDistrib;
    vector<string> nodes;
    helper.getDistributions(appDistrib, nodes);

    if (nodes.empty())
    {
        response();
        return;
    }

    Ice::Identity id;
    id.category = current.id.category;
    id.name = Ice::generateUUID();

    shared_ptr<PatcherFeedbackAggregator> feedback =
        newPatcherFeedback(response, exception, id, _traceLevels, "application", name, static_cast<int>(nodes.size()));

    for (vector<string>::const_iterator p = nodes.begin(); p != nodes.end(); ++p)
    {
        try
        {
            if (_traceLevels->patch > 0)
            {
                Ice::Trace out(_traceLevels->logger, _traceLevels->patchCat);
                out << "started patching of application `" << name << "' on node `" << *p << "'";
            }

            shared_ptr<NodeEntry> node = _database->getNode(*p);
            Resolver resolve(node->getInfo(), _database->getCommunicator());
            DistributionDescriptor desc = resolve(appDistrib);
            auto intAppDistrib = make_shared<InternalDistributionDescriptor>(desc.icepatch, desc.directories);
            node->getSession()->patch(feedback, name, "", intAppDistrib, shutdown);
        }
        catch (const NodeNotExistException&)
        {
            feedback->failed(*p, "node doesn't exist");
        }
        catch (const NodeUnreachableException& e)
        {
            feedback->failed(*p, "node is unreachable: " + e.reason);
        }
        catch (const Ice::Exception& e)
        {
            ostringstream os;
            os << e;
            feedback->failed(*p, "node is unreachable:\n" + os.str());
        }
    }
}

ApplicationInfo
AdminI::getApplicationInfo(string name, const Current&) const
{
    return _database->getApplicationInfo(std::move(name));
}

ApplicationDescriptor
AdminI::getDefaultApplicationDescriptor(const Current& current) const
{
    auto properties = current.adapter->getCommunicator()->getProperties();
    string path = properties->getProperty("IceGrid.Registry.DefaultTemplates");
    if (path.empty())
    {
        throw DeploymentException("no default templates configured, you need to set "
                                  "IceGrid.Registry.DefaultTemplates in the IceGrid registry configuration.");
    }

    ApplicationDescriptor desc;
    try
    {
        desc = DescriptorParser::parseDescriptor(path, current.adapter->getCommunicator());
    }
    catch (const IceXML::ParserException& ex)
    {
        throw DeploymentException("can't parse default templates:\n" + ex.reason());
    }
    desc.name = "";
    if (!desc.nodes.empty())
    {
        Ice::Warning warn(_traceLevels->logger);
        warn << "default application descriptor:\nnode definitions are not allowed.";
        desc.nodes.clear();
    }
    if (!desc.distrib.icepatch.empty() || !desc.distrib.directories.empty())
    {
        Ice::Warning warn(_traceLevels->logger);
        warn << "default application descriptor:\ndistribution is not allowed.";
        desc.distrib = DistributionDescriptor();
    }
    if (!desc.replicaGroups.empty())
    {
        Ice::Warning warn(_traceLevels->logger);
        warn << "default application descriptor:\nreplica group definitions are not allowed.";
        desc.replicaGroups.clear();
    }
    if (!desc.description.empty())
    {
        Ice::Warning warn(_traceLevels->logger);
        warn << "default application descriptor:\ndescription is not allowed.";
        desc.description = "";
    }
    if (!desc.variables.empty())
    {
        Ice::Warning warn(_traceLevels->logger);
        warn << "default application descriptor:\nvariable definitions are not allowed.";
        desc.variables.clear();
    }
    return desc;
}

Ice::StringSeq
AdminI::getAllApplicationNames(const Current&) const
{
    return _database->getAllApplications();
}

ServerInfo
AdminI::getServerInfo(string id, const Current&) const
{
    return _database->getServer(std::move(id))->getInfo(true);
}

ServerState
AdminI::getServerState(string id, const Current&) const
{
    ServerProxyWrapper proxy(_database, std::move(id));
    return proxy.invoke(&ServerPrx::getState);
}

int
AdminI::getServerPid(string id, const Current&) const
{
    ServerProxyWrapper proxy(_database, std::move(id));
    return proxy.invoke(&ServerPrx::getPid);
}

string
AdminI::getServerAdminCategory(const Current&) const
{
    return _registry->getServerAdminCategory();
}

ObjectPrxPtr
AdminI::getServerAdmin(string id, const Current& current) const
{
    ServerProxyWrapper proxy(_database, id); // Ensure that the server exists and loaded on the node.

    return current.adapter->createProxy({id, _registry->getServerAdminCategory()});
}

void
AdminI::startServerAsync(string id, function<void()> response, function<void(exception_ptr)> exception, const Current&)
{
    ServerProxyWrapper proxy(_database, std::move(id));
    proxy.useActivationTimeout();

    proxy.invokeAsync([](const auto& prx, auto... args) { prx->startAsync(args...); }, std::move(response),
                      std::move(exception));
}

void
AdminI::stopServerAsync(string id, function<void()> response, function<void(exception_ptr)> exception, const Current&)
{
    ServerProxyWrapper proxy(_database, id);
    proxy.useDeactivationTimeout();

    //
    // Since the server might take a while to be deactivated, we use AMI.
    //
    proxy.invokeAsync([](const auto& prx, auto... args) { prx->stopAsync(args...); }, response,
                      [response, exception = std::move(exception)](exception_ptr ex)
                      {
                          try
                          {
                              rethrow_exception(ex);
                          }
                          catch (const Ice::TimeoutException&)
                          {
                              response();
                          }
                          catch (const Ice::Exception&)
                          {
                              exception(current_exception());
                          }
                      });
}

void
AdminI::patchServerAsync(string id,
                         bool shutdown,
                         function<void()> response,
                         function<void(exception_ptr)> exception,
                         const Current& current)
{
    ServerInfo info = _database->getServer(id)->getInfo();
    ApplicationInfo appInfo = _database->getApplicationInfo(info.application);
    ApplicationHelper helper(current.adapter->getCommunicator(), appInfo.descriptor);
    DistributionDescriptor appDistrib;
    vector<string> nodes;
    helper.getDistributions(appDistrib, nodes, id);

    if (appDistrib.icepatch.empty() && nodes.empty())
    {
        response();
        return;
    }

    assert(nodes.size() == 1);

    Ice::Identity identity;
    identity.category = current.id.category;
    identity.name = Ice::generateUUID();

    shared_ptr<PatcherFeedbackAggregator> feedback =
        newPatcherFeedback(response, exception, identity, _traceLevels, "server", id, static_cast<int>(nodes.size()));

    vector<string>::const_iterator p = nodes.begin();
    try
    {
        if (_traceLevels->patch > 0)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->patchCat);
            out << "started patching of server `" << id << "' on node `" << *p << "'";
        }

        shared_ptr<NodeEntry> node = _database->getNode(*p);
        Resolver resolve(node->getInfo(), _database->getCommunicator());
        DistributionDescriptor desc = resolve(appDistrib);
        shared_ptr<InternalDistributionDescriptor> intAppDistrib =
            make_shared<InternalDistributionDescriptor>(desc.icepatch, desc.directories);
        node->getSession()->patch(feedback, info.application, id, intAppDistrib, shutdown);
    }
    catch (const NodeNotExistException&)
    {
        feedback->failed(*p, "node doesn't exist");
    }
    catch (const NodeUnreachableException& e)
    {
        feedback->failed(*p, "node is unreachable: " + e.reason);
    }
    catch (const Ice::Exception& e)
    {
        ostringstream os;
        os << e;
        feedback->failed(*p, "node is unreachable:\n" + os.str());
    }
}

void
AdminI::sendSignal(string id, string signal, const Current&)
{
    ServerProxyWrapper proxy(_database, std::move(id));
    proxy.invoke(&ServerPrx::sendSignal, std::move(signal));
}

StringSeq
AdminI::getAllServerIds(const Current&) const
{
    return _database->getServerCache().getAll("");
}

void
AdminI::enableServer(string id, bool enable, const Ice::Current&)
{
    ServerProxyWrapper proxy(_database, std::move(id));
    proxy.invoke(&ServerPrx::setEnabled, std::move(enable));
}

bool
AdminI::isServerEnabled(string id, const Ice::Current&) const
{
    ServerProxyWrapper proxy(_database, std::move(id));
    return proxy.invoke(&ServerPrx::isEnabled);
}

AdapterInfoSeq
AdminI::getAdapterInfo(string id, const Current&) const
{
    return _database->getAdapterInfo(id);
}

void
AdminI::removeAdapter(string adapterId, const Ice::Current&)
{
    checkIsReadOnly();
    _database->removeAdapter(adapterId);
}

StringSeq
AdminI::getAllAdapterIds(const Current&) const
{
    return _database->getAllAdapters();
}

void
AdminI::addObject(Ice::ObjectPrxPtr proxy, const ::Ice::Current& current)
{
    checkIsReadOnly();

    if (!proxy)
    {
        throw DeploymentException("proxy is null");
    }

    try
    {
        addObjectWithType(proxy, proxy->ice_id(), current);
    }
    catch (const Ice::LocalException& ex)
    {
        ostringstream os;
        os << "failed to invoke ice_id() on proxy `" << proxy << "':\n" << ex;
        throw DeploymentException(os.str());
    }
}

void
AdminI::updateObject(Ice::ObjectPrxPtr proxy, const ::Ice::Current&)
{
    checkIsReadOnly();

    if (!proxy)
    {
        throw DeploymentException("proxy is null");
    }

    const Ice::Identity id = proxy->ice_getIdentity();
    if (id.category == _database->getInstanceName())
    {
        throw DeploymentException("updating object `" + _database->getCommunicator()->identityToString(id) +
                                  "' is not allowed:\nobjects with identity category `" + id.category +
                                  "' are managed by IceGrid");
    }
    _database->updateObject(proxy);
}

void
AdminI::addObjectWithType(Ice::ObjectPrxPtr proxy, string type, const ::Ice::Current&)
{
    checkIsReadOnly();

    if (!proxy)
    {
        throw DeploymentException("proxy is null");
    }

    const Ice::Identity id = proxy->ice_getIdentity();
    if (id.category == _database->getInstanceName())
    {
        throw DeploymentException("adding object `" + _database->getCommunicator()->identityToString(id) +
                                  "' is not allowed:\nobjects with identity category `" + id.category +
                                  "' are managed by IceGrid");
    }

    ObjectInfo info;
    info.proxy = proxy;
    info.type = type;
    _database->addObject(info);
}

void
AdminI::removeObject(Ice::Identity id, const Ice::Current&)
{
    checkIsReadOnly();
    if (id.category == _database->getInstanceName())
    {
        throw DeploymentException("removing object `" + _database->getCommunicator()->identityToString(id) +
                                  "' is not allowed:\nobjects with identity category `" + id.category +
                                  "' are managed by IceGrid");
    }
    _database->removeObject(id);
}

ObjectInfo
AdminI::getObjectInfo(Ice::Identity id, const Ice::Current&) const
{
    return _database->getObjectInfo(std::move(id));
}

ObjectInfoSeq
AdminI::getObjectInfosByType(string type, const Ice::Current&) const
{
    return _database->getObjectInfosByType(std::move(type));
}

ObjectInfoSeq
AdminI::getAllObjectInfos(string expression, const Ice::Current&) const
{
    return _database->getAllObjectInfos(std::move(expression));
}

NodeInfo
AdminI::getNodeInfo(string name, const Ice::Current&) const
{
    return toNodeInfo(_database->getNode(name)->getInfo());
}

ObjectPrxPtr
AdminI::getNodeAdmin(string name, const Current& current) const
{
    //
    // Check if the node exists
    //
    _database->getNode(name);

    return current.adapter->createProxy({name, _registry->getNodeAdminCategory()});
}

bool
AdminI::pingNode(string name, const Current&) const
{
    try
    {
        _database->getNode(name)->getProxy()->ice_ping();
        return true;
    }
    catch (const NodeUnreachableException&)
    {
        return false;
    }
    catch (const Ice::ObjectNotExistException&)
    {
        throw NodeNotExistException();
    }
    catch (const Ice::LocalException&)
    {
        return false;
    }
}

LoadInfo
AdminI::getNodeLoad(string name, const Current&) const
{
    try
    {
        return _database->getNode(name)->getProxy()->getLoad();
    }
    catch (const Ice::ObjectNotExistException&)
    {
        throw NodeNotExistException();
    }
    catch (const Ice::LocalException& ex)
    {
        ostringstream os;
        os << ex;
        throw NodeUnreachableException(name, os.str());
    }
}

int
AdminI::getNodeProcessorSocketCount(string name, const Current&) const
{
    try
    {
        return _database->getNode(name)->getProxy()->getProcessorSocketCount();
    }
    catch (const Ice::OperationNotExistException&)
    {
        return 0; // Not supported.
    }
    catch (const Ice::ObjectNotExistException&)
    {
        throw NodeNotExistException(name);
        return 0;
    }
    catch (const Ice::LocalException& ex)
    {
        ostringstream os;
        os << ex;
        throw NodeUnreachableException(name, os.str());
        return 0;
    }
}

void
AdminI::shutdownNode(string name, const Current&)
{
    try
    {
        _database->getNode(name)->getProxy()->shutdown();
    }
    catch (const Ice::ObjectNotExistException&)
    {
        throw NodeNotExistException(name);
    }
    catch (const Ice::LocalException& ex)
    {
        ostringstream os;
        os << ex;
        throw NodeUnreachableException(name, os.str());
    }
}

string
AdminI::getNodeHostname(string name, const Current&) const
{
    try
    {
        return _database->getNode(name)->getInfo()->hostname;
    }
    catch (const Ice::ObjectNotExistException&)
    {
        throw NodeNotExistException(name);
    }
    catch (const Ice::LocalException& ex)
    {
        ostringstream os;
        os << ex;
        throw NodeUnreachableException(name, os.str());
    }
}

StringSeq
AdminI::getAllNodeNames(const Current&) const
{
    return _database->getNodeCache().getAll("");
}

RegistryInfo
AdminI::getRegistryInfo(string name, const Ice::Current&) const
{
    if (name == _registry->getName())
    {
        return _registry->getInfo();
    }
    else
    {
        return toRegistryInfo(_database->getReplica(name)->getInfo());
    }
}

ObjectPrxPtr
AdminI::getRegistryAdmin(string name, const Current& current) const
{
    if (name != _registry->getName())
    {
        //
        // Check if the replica exists
        //
        _database->getReplica(name);
    }

    return current.adapter->createProxy({name, _registry->getReplicaAdminCategory()});
}

bool
AdminI::pingRegistry(string name, const Current&) const
{
    if (name == _registry->getName())
    {
        return true;
    }

    try
    {
        _database->getReplica(name)->getProxy()->ice_ping();
        return true;
    }
    catch (const Ice::ObjectNotExistException&)
    {
        throw RegistryNotExistException();
    }
    catch (const Ice::LocalException&)
    {
        return false;
    }
}

void
AdminI::shutdownRegistry(string name, const Current&)
{
    if (name == _registry->getName())
    {
        _registry->shutdown();
        return;
    }

    try
    {
        _database->getReplica(name)->getProxy()->shutdown();
    }
    catch (const Ice::ObjectNotExistException&)
    {
        throw RegistryNotExistException(name);
    }
    catch (const Ice::LocalException& ex)
    {
        ostringstream os;
        os << ex;
        throw RegistryUnreachableException(name, os.str());
    }
}

StringSeq
AdminI::getAllRegistryNames(const Current&) const
{
    Ice::StringSeq replicas = _database->getReplicaCache().getAll("");
    replicas.push_back(_registry->getName());
    return replicas;
}

void
AdminI::shutdown(const Current&)
{
    _registry->shutdown();
}

void
AdminI::checkIsReadOnly() const
{
    if (_database->isReadOnly())
    {
        throw DeploymentException("this operation is not allowed on a slave or read-only master registry.");
    }
}
