// Copyright (c) ZeroC, Inc.

#include "AdminI.h"
#include "../Ice/TraceUtil.h"
#include "AdminSessionI.h"
#include "Database.h"
#include "DescriptorHelper.h"
#include "DescriptorParser.h"
#include "Ice/Ice.h"
#include "Ice/LoggerUtil.h"
#include "Ice/UUID.h"
#include "NodeSessionI.h"
#include "RegistryI.h"
#include "SynchronizationException.h"
#include "Util.h"

using namespace std;
using namespace std::chrono;
using namespace Ice;
using namespace IceGrid;

namespace
{
    class ServerProxyWrapper
    {
    public:
        static ServerProxyWrapper create(const shared_ptr<Database>& database, string id)
        {
            try
            {
                chrono::seconds activationTimeout;
                chrono::seconds deactivationTimeout;
                string node;
                ServerPrx proxy =
                    database->getServer(id)->getProxy(activationTimeout, deactivationTimeout, node, false, 5s);

                return {std::move(id), std::move(proxy), activationTimeout, deactivationTimeout, std::move(node)};
            }
            catch (const SynchronizationException&)
            {
                throw DeploymentException("server is being updated");
            }
        }

        ServerProxyWrapper(const ServerProxyWrapper& wrapper) = delete;

        template<typename Func, typename... Args> auto invoke(Func&& f, Args&&... args)
        {
            try
            {
                return std::invoke(
                    std::forward<Func>(f),
                    _proxy,
                    std::forward<Args>(args)...,
                    ::Ice::noExplicitContext);
            }
            catch (const Ice::Exception&)
            {
                handleException(current_exception());
                throw; // keep compiler happy
            }
        }

        template<typename Func>
        auto invokeAsync(Func&& f, function<void()> response, function<void(exception_ptr)> exception)
        {
            return std::invoke(
                std::forward<Func>(f),
                _proxy,
                std::move(response),
                [this, exception = std::move(exception)](exception_ptr ex)
                {
                    try
                    {
                        handleException(ex);
                    }
                    catch (...)
                    {
                        exception(current_exception());
                    }
                },
                nullptr,
                Ice::noExplicitContext);
        }

        void useActivationTimeout() { _proxy = _proxy->ice_invocationTimeout(_activationTimeout); }

        void useDeactivationTimeout() { _proxy = _proxy->ice_invocationTimeout(_deactivationTimeout); }

        ServerPrx* operator->() { return &_proxy; }

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
        ServerProxyWrapper(
            string id,
            ServerPrx proxy,
            chrono::seconds activationTimeout,
            chrono::seconds deactivationTimeout,
            string node)
            : _id(std::move(id)),
              _proxy(std::move(proxy)),
              _activationTimeout(activationTimeout),
              _deactivationTimeout(deactivationTimeout),
              _node(std::move(node))
        {
        }

        string _id;
        ServerPrx _proxy;
        chrono::seconds _activationTimeout;
        chrono::seconds _deactivationTimeout;
        string _node;
    };
}

AdminI::AdminI(
    const shared_ptr<Database>& database,
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
    _database->removeApplication(name, _session.get());
}

void
AdminI::instantiateServer(string app, string node, ServerInstanceDescriptor desc, const Current&)
{
    checkIsReadOnly();
    _database->instantiateServer(app, node, desc, _session.get());
}

ApplicationInfo
AdminI::getApplicationInfo(string name, const Current&) const
{
    return _database->getApplicationInfo(name);
}

ApplicationDescriptor
AdminI::getDefaultApplicationDescriptor(const Current& current) const
{
    auto properties = current.adapter->getCommunicator()->getProperties();
    string path = properties->getIceProperty("IceGrid.Registry.DefaultTemplates");
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
    catch (const XMLParserException& ex)
    {
        throw DeploymentException("can't parse default templates:\n" + string{ex.what()});
    }
    desc.name = "";
    if (!desc.nodes.empty())
    {
        Ice::Warning warn(_traceLevels->logger);
        warn << "default application descriptor:\nnode definitions are not allowed.";
        desc.nodes.clear();
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
    return _database->getServer(id)->getInfo(true);
}

ServerState
AdminI::getServerState(string id, const Current&) const
{
    auto proxy = ServerProxyWrapper::create(_database, std::move(id));
    return proxy.invoke(&ServerPrx::getState);
}

int
AdminI::getServerPid(string id, const Current&) const
{
    auto proxy = ServerProxyWrapper::create(_database, std::move(id));
    return proxy.invoke(&ServerPrx::getPid);
}

string
AdminI::getServerAdminCategory(const Current&) const
{
    return _registry->getServerAdminCategory();
}

optional<ObjectPrx>
AdminI::getServerAdmin(string id, const Current& current) const
{
    auto proxy = ServerProxyWrapper::create(_database, id); // Ensure that the server exists and loaded on the node.

    return current.adapter->createProxy({id, _registry->getServerAdminCategory()});
}

void
AdminI::startServerAsync(string id, function<void()> response, function<void(exception_ptr)> exception, const Current&)
{
    auto proxy = ServerProxyWrapper::create(_database, std::move(id));
    proxy.useActivationTimeout();

    proxy.invokeAsync(
        [](const auto& prx, auto&&... args) { prx->startAsync(std::forward<decltype(args)>(args)...); },
        std::move(response),
        std::move(exception));
}

void
AdminI::stopServerAsync(string id, function<void()> response, function<void(exception_ptr)> exception, const Current&)
{
    auto proxy = ServerProxyWrapper::create(_database, std::move(id));
    proxy.useDeactivationTimeout();

    //
    // Since the server might take a while to be deactivated, we use AMI.
    //
    proxy.invokeAsync(
        [](const auto& prx, auto&&... args) { prx->stopAsync(std::forward<decltype(args)>(args)...); },
        response,
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
AdminI::sendSignal(string id, string signal, const Current&)
{
    auto proxy = ServerProxyWrapper::create(_database, std::move(id));
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
    auto proxy = ServerProxyWrapper::create(_database, std::move(id));
    proxy.invoke(&ServerPrx::setEnabled, enable);
}

bool
AdminI::isServerEnabled(string id, const Ice::Current&) const
{
    auto proxy = ServerProxyWrapper::create(_database, std::move(id));
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
AdminI::addObject(optional<Ice::ObjectPrx> proxy, const Ice::Current& current)
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
        os << "failed to invoke ice_id() on proxy '" << proxy << "':\n" << ex;
        throw DeploymentException(os.str());
    }
}

void
AdminI::updateObject(optional<Ice::ObjectPrx> proxy, const Ice::Current&)
{
    checkIsReadOnly();

    if (!proxy)
    {
        throw DeploymentException("proxy is null");
    }

    const Ice::Identity id = proxy->ice_getIdentity();
    if (id.category == _database->getInstanceName())
    {
        throw DeploymentException(
            "updating object '" + _database->getCommunicator()->identityToString(id) +
            "' is not allowed:\nobjects with identity category '" + id.category + "' are managed by IceGrid");
    }
    _database->updateObject(*proxy);
}

void
AdminI::addObjectWithType(optional<Ice::ObjectPrx> proxy, string type, const Ice::Current&)
{
    checkIsReadOnly();

    if (!proxy)
    {
        throw DeploymentException("proxy is null");
    }

    const Ice::Identity id = proxy->ice_getIdentity();
    if (id.category == _database->getInstanceName())
    {
        throw DeploymentException(
            "adding object '" + _database->getCommunicator()->identityToString(id) +
            "' is not allowed:\nobjects with identity category '" + id.category + "' are managed by IceGrid");
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
        throw DeploymentException(
            "removing object '" + _database->getCommunicator()->identityToString(id) +
            "' is not allowed:\nobjects with identity category '" + id.category + "' are managed by IceGrid");
    }
    _database->removeObject(id);
}

ObjectInfo
AdminI::getObjectInfo(Ice::Identity id, const Ice::Current&) const
{
    return _database->getObjectInfo(id);
}

ObjectInfoSeq
AdminI::getObjectInfosByType(string type, const Ice::Current&) const
{
    return _database->getObjectInfosByType(type);
}

ObjectInfoSeq
AdminI::getAllObjectInfos(string expression, const Ice::Current&) const
{
    return _database->getAllObjectInfos(expression);
}

NodeInfo
AdminI::getNodeInfo(string name, const Ice::Current&) const
{
    return toNodeInfo(_database->getNode(name)->getInfo());
}

optional<ObjectPrx>
AdminI::getNodeAdmin(string name, const Current& current) const
{
    //
    // Check if the node exists
    //
    auto ignored = _database->getNode(name);

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

optional<ObjectPrx>
AdminI::getRegistryAdmin(string name, const Current& current) const
{
    if (name != _registry->getName())
    {
        //
        // Check if the replica exists
        //
        auto ignored = _database->getReplica(name);
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
