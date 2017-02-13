// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/UUID.h>

#include <Ice/Ice.h>
#include <Ice/LoggerUtil.h>
#include <Ice/TraceUtil.h>
#include <Ice/SliceChecksums.h>

#include <IceGrid/AdminI.h>
#include <IceGrid/RegistryI.h>
#include <IceGrid/Database.h>
#include <IceGrid/Util.h>
#include <IceGrid/DescriptorParser.h>
#include <IceGrid/DescriptorHelper.h>
#include <IceGrid/AdminSessionI.h>
#include <IceGrid/NodeSessionI.h>


using namespace std;
using namespace Ice;
using namespace IceGrid;

namespace IceGrid
{

class ServerProxyWrapper
{
public:

    ServerProxyWrapper(const DatabasePtr& database, const string& id) : _id(id)
    {
        try
        {
            _proxy = database->getServer(_id)->getProxy(_activationTimeout, _deactivationTimeout, _node, false, 5);
        }
        catch(const SynchronizationException&)
        {
            throw DeploymentException("server is being updated");
        }
    }

    ServerProxyWrapper(const ServerProxyWrapper& wrapper) :
        _id(wrapper._id),
        _proxy(wrapper._proxy),
        _activationTimeout(wrapper._activationTimeout),
        _deactivationTimeout(wrapper._deactivationTimeout),
        _node(wrapper._node)
    {
    }

    void
    useActivationTimeout()
    {
        _proxy = ServerPrx::uncheckedCast(_proxy->ice_invocationTimeout(_activationTimeout * 1000));
    }

    void
    useDeactivationTimeout()
    {
        _proxy = ServerPrx::uncheckedCast(_proxy->ice_invocationTimeout(_deactivationTimeout * 1000));
    }

    IceProxy::IceGrid::Server*
    operator->() const
    {
        return _proxy.get();
    }

    void
    handleException(const Ice::Exception& ex) const
    {
        try
        {
            ex.ice_throw();
        }
        catch(const Ice::UserException&)
        {
            throw;
        }
        catch(const Ice::ObjectNotExistException&)
        {
            throw ServerNotExistException(_id);
        }
        catch(const Ice::LocalException& e)
        {
            ostringstream os;
            os << e;
            throw NodeUnreachableException(_node, os.str());
        }
    }

private:

    string _id;
    ServerPrx _proxy;
    int _activationTimeout;
    int _deactivationTimeout;
    string _node;
};

template<class AmdCB>
class AMDPatcherFeedbackAggregator : public PatcherFeedbackAggregator
{
public:

    AMDPatcherFeedbackAggregator(const AmdCB& cb,
                                 Ice::Identity id,
                                 const TraceLevelsPtr& traceLevels,
                                 const string& type,
                                 const string& name,
                                 int nodeCount) :
        PatcherFeedbackAggregator(id, traceLevels, type, name, nodeCount),
        _cb(cb)
    {
    }

private:

    void
    response()
    {
        _cb->ice_response();
    }

    void
    exception(const Ice::Exception& ex)
    {
        _cb->ice_exception(ex);
    }

    const AmdCB _cb;
};


template<typename AmdCB> PatcherFeedbackAggregatorPtr
static newPatcherFeedback(const AmdCB& cb,
                          Ice::Identity id,
                          const TraceLevelsPtr& traceLevels,
                          const string& type,
                          const string& name,
                          int nodeCount)
{
    return new AMDPatcherFeedbackAggregator<AmdCB>(cb, id, traceLevels, type, name, nodeCount);
}

}

AdminI::AdminI(const DatabasePtr& database, const RegistryIPtr& registry, const AdminSessionIPtr& session) :
    _database(database),
    _registry(registry),
    _traceLevels(_database->getTraceLevels()),
    _session(session)
{
}

AdminI::~AdminI()
{
}


void
AdminI::addApplication(const ApplicationDescriptor& descriptor, const Current&)
{
    checkIsReadOnly();

    ApplicationInfo info;
    info.createTime = info.updateTime = IceUtil::Time::now().toMilliSeconds();
    info.createUser = info.updateUser = _session->getId();
    info.descriptor = descriptor;
    info.revision = 1;
    info.uuid = IceUtil::generateUUID();

    _database->addApplication(info, _session.get());
}

void
AdminI::syncApplication(const ApplicationDescriptor& descriptor, const Current&)
{
    checkIsReadOnly();
    _database->syncApplicationDescriptor(descriptor, false, _session.get());
}

void
AdminI::updateApplication(const ApplicationUpdateDescriptor& descriptor, const Current&)
{
    checkIsReadOnly();

    ApplicationUpdateInfo update;
    update.updateTime = IceUtil::Time::now().toMilliSeconds();
    update.updateUser = _session->getId();
    update.descriptor = descriptor;
    update.revision = -1; // The database will set it.
    _database->updateApplication(update, false, _session.get());
}

void
AdminI::syncApplicationWithoutRestart(const ApplicationDescriptor& descriptor, const Current&)
{
    checkIsReadOnly();
    _database->syncApplicationDescriptor(descriptor, true, _session.get());
}

void
AdminI::updateApplicationWithoutRestart(const ApplicationUpdateDescriptor& descriptor, const Current&)
{
    checkIsReadOnly();

    ApplicationUpdateInfo update;
    update.updateTime = IceUtil::Time::now().toMilliSeconds();
    update.updateUser = _session->getId();
    update.descriptor = descriptor;
    update.revision = -1; // The database will set it.
    _database->updateApplication(update, true, _session.get());
}

void
AdminI::removeApplication(const string& name, const Current&)
{
    checkIsReadOnly();
    _database->removeApplication(name, _session.get());
}

void
AdminI::instantiateServer(const string& app, const string& node, const ServerInstanceDescriptor& desc, const Current&)
{
    checkIsReadOnly();
    _database->instantiateServer(app, node, desc, _session.get());
}

void
AdminI::patchApplication_async(const AMD_Admin_patchApplicationPtr& amdCB,
                               const string& name,
                               bool shutdown,
                               const Current& current)
{
    ApplicationHelper helper(current.adapter->getCommunicator(), _database->getApplicationInfo(name).descriptor);
    DistributionDescriptor appDistrib;
    vector<string> nodes;
    helper.getDistributions(appDistrib, nodes);

    if(nodes.empty())
    {
        amdCB->ice_response();
        return;
    }

    Ice::Identity id;
    id.category = current.id.category;
    id.name = IceUtil::generateUUID();

    PatcherFeedbackAggregatorPtr feedback =
        newPatcherFeedback(amdCB, id, _traceLevels, "application", name, static_cast<int>(nodes.size()));

    for(vector<string>::const_iterator p = nodes.begin(); p != nodes.end(); ++p)
    {
        try
        {
            if(_traceLevels->patch > 0)
            {
                Ice::Trace out(_traceLevels->logger, _traceLevels->patchCat);
                out << "started patching of application `" << name << "' on node `" << *p << "'";
            }

            NodeEntryPtr node = _database->getNode(*p);
            Resolver resolve(node->getInfo(), _database->getCommunicator());
            DistributionDescriptor desc = resolve(appDistrib);
            InternalDistributionDescriptorPtr intAppDistrib = new InternalDistributionDescriptor(desc.icepatch,
                                                                                                 desc.directories);
            node->getSession()->patch(feedback, name, "", intAppDistrib, shutdown);
        }
        catch(const NodeNotExistException&)
        {
            feedback->failed(*p, "node doesn't exist");
        }
        catch(const NodeUnreachableException& e)
        {
            feedback->failed(*p, "node is unreachable: " + e.reason);
        }
        catch(const Ice::Exception& e)
        {
            ostringstream os;
            os << e;
            feedback->failed(*p, "node is unreachable:\n" + os.str());
        }
    }
}

ApplicationInfo
AdminI::getApplicationInfo(const string& name, const Current&) const
{
    return _database->getApplicationInfo(name);
}

ApplicationDescriptor
AdminI::getDefaultApplicationDescriptor(const Current& current) const
{
    Ice::PropertiesPtr properties = current.adapter->getCommunicator()->getProperties();
    string path = properties->getProperty("IceGrid.Registry.DefaultTemplates");
    if(path.empty())
    {
        throw DeploymentException("no default templates configured, you need to set "
                                  "IceGrid.Registry.DefaultTemplates in the IceGrid registry configuration.");
    }

    ApplicationDescriptor desc;
    try
    {
        desc = DescriptorParser::parseDescriptor(path, current.adapter->getCommunicator());
    }
    catch(const IceXML::ParserException& ex)
    {
        throw DeploymentException("can't parse default templates:\n" + ex.reason());
    }
    desc.name = "";
    if(!desc.nodes.empty())
    {
        Ice::Warning warn(_traceLevels->logger);
        warn << "default application descriptor:\nnode definitions are not allowed.";
        desc.nodes.clear();
    }
    if(!desc.distrib.icepatch.empty() || !desc.distrib.directories.empty())
    {
        Ice::Warning warn(_traceLevels->logger);
        warn << "default application descriptor:\ndistribution is not allowed.";
        desc.distrib = DistributionDescriptor();
    }
    if(!desc.replicaGroups.empty())
    {
        Ice::Warning warn(_traceLevels->logger);
        warn << "default application descriptor:\nreplica group definitions are not allowed.";
        desc.replicaGroups.clear();
    }
    if(!desc.description.empty())
    {
        Ice::Warning warn(_traceLevels->logger);
        warn << "default application descriptor:\ndescription is not allowed.";
        desc.description = "";
    }
    if(!desc.variables.empty())
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
AdminI::getServerInfo(const string& id, const Current&) const
{
    return _database->getServer(id)->getInfo(true);
}

ServerState
AdminI::getServerState(const string& id, const Current&) const
{
    ServerProxyWrapper proxy(_database, id);
    try
    {
        return proxy->getState();
    }
    catch(const Ice::Exception& ex)
    {
        proxy.handleException(ex);
        return Inactive;
    }
}

Ice::Int
AdminI::getServerPid(const string& id, const Current&) const
{
    ServerProxyWrapper proxy(_database, id);
    try
    {
        return proxy->getPid();
    }
    catch(const Ice::Exception& ex)
    {
        proxy.handleException(ex);
        return 0;
    }
}

string
AdminI::getServerAdminCategory(const Current&) const
{
    return _registry->getServerAdminCategory();
}

ObjectPrx
AdminI::getServerAdmin(const string& id, const Current& current) const
{
    ServerProxyWrapper proxy(_database, id); // Ensure that the server exists and loaded on the node.

    Ice::Identity adminId;
    adminId.name = id;
    adminId.category = _registry->getServerAdminCategory();
    return current.adapter->createProxy(adminId);
}

namespace
{

class StartCB : virtual public IceUtil::Shared
{
public:

    StartCB(const ServerProxyWrapper& proxy, const AMD_Admin_startServerPtr& amdCB) : _proxy(proxy), _amdCB(amdCB)
    {
    }

    virtual void
    response()
    {
        _amdCB->ice_response();
    }

    virtual void
    exception(const Ice::Exception& ex)
    {
        try
        {
            _proxy.handleException(ex);
            assert(false);
        }
        catch(const Ice::Exception& ex)
        {
            _amdCB->ice_exception(ex);
        }
    }

private:

    const ServerProxyWrapper _proxy;
    const AMD_Admin_startServerPtr _amdCB;
};

}


void
AdminI::startServer_async(const AMD_Admin_startServerPtr& amdCB, const string& id, const Current&)
{
    ServerProxyWrapper proxy(_database, id);
    proxy.useActivationTimeout();

    //
    // Since the server might take a while to be activated, we use AMI.
    //
    proxy->begin_start(newCallback_Server_start(new StartCB(proxy, amdCB),
                                                &StartCB::response,
                                                &StartCB::exception));
}

namespace
{

class StopCB : virtual public IceUtil::Shared
{
public:

    StopCB(const ServerProxyWrapper& proxy, const AMD_Admin_stopServerPtr& amdCB) : _proxy(proxy), _amdCB(amdCB)
    {
    }

    virtual void
    response()
    {
        _amdCB->ice_response();
    }

    virtual void
    exception(const Ice::Exception& ex)
    {
        try
        {
            _proxy.handleException(ex);
            assert(false);
        }
        catch(const Ice::TimeoutException&)
        {
            _amdCB->ice_response();
        }
        catch(const Ice::Exception& ex)
        {
            _amdCB->ice_exception(ex);
        }
    }

private:

    const ServerProxyWrapper _proxy;
    const AMD_Admin_stopServerPtr _amdCB;
};

}

void
AdminI::stopServer_async(const AMD_Admin_stopServerPtr& amdCB, const string& id, const Current&)
{
    ServerProxyWrapper proxy(_database, id);
    proxy.useDeactivationTimeout();

    //
    // Since the server might take a while to be deactivated, we use AMI.
    //
    proxy->begin_stop(newCallback_Server_stop(new StopCB(proxy, amdCB),
                                              &StopCB::response,
                                              &StopCB::exception));
}

void
AdminI::patchServer_async(const AMD_Admin_patchServerPtr& amdCB, const string& id, bool shutdown,
                          const Current& current)
{
    ServerInfo info = _database->getServer(id)->getInfo();
    ApplicationInfo appInfo = _database->getApplicationInfo(info.application);
    ApplicationHelper helper(current.adapter->getCommunicator(), appInfo.descriptor);
    DistributionDescriptor appDistrib;
    vector<string> nodes;
    helper.getDistributions(appDistrib, nodes, id);

    if(appDistrib.icepatch.empty() && nodes.empty())
    {
        amdCB->ice_response();
        return;
    }

    assert(nodes.size() == 1);

    Ice::Identity identity;
    identity.category = current.id.category;
    identity.name = IceUtil::generateUUID();

    PatcherFeedbackAggregatorPtr feedback =
        newPatcherFeedback(amdCB, identity, _traceLevels, "server", id, static_cast<int>(nodes.size()));

    vector<string>::const_iterator p = nodes.begin();
    try
    {
        if(_traceLevels->patch > 0)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->patchCat);
            out << "started patching of server `" << id << "' on node `" << *p << "'";
        }

        NodeEntryPtr node = _database->getNode(*p);
        Resolver resolve(node->getInfo(), _database->getCommunicator());
        DistributionDescriptor desc = resolve(appDistrib);
        InternalDistributionDescriptorPtr intAppDistrib = new InternalDistributionDescriptor(desc.icepatch,
                                                                                             desc.directories);
        node->getSession()->patch(feedback, info.application, id, intAppDistrib, shutdown);
    }
    catch(const NodeNotExistException&)
    {
        feedback->failed(*p, "node doesn't exist");
    }
    catch(const NodeUnreachableException& e)
    {
        feedback->failed(*p, "node is unreachable: " + e.reason);
    }
    catch(const Ice::Exception& e)
    {
        ostringstream os;
        os << e;
        feedback->failed(*p, "node is unreachable:\n" + os.str());
    }
}

void
AdminI::sendSignal(const string& id, const string& signal, const Current&)
{
    ServerProxyWrapper proxy(_database, id);
    try
    {
        proxy->sendSignal(signal);
    }
    catch(const Ice::Exception& ex)
    {
        proxy.handleException(ex);
    }
}

StringSeq
AdminI::getAllServerIds(const Current&) const
{
    return _database->getServerCache().getAll("");
}

void
AdminI::enableServer(const string& id, bool enable, const Ice::Current&)
{
    ServerProxyWrapper proxy(_database, id);
    try
    {
        proxy->setEnabled(enable);
    }
    catch(const Ice::Exception& ex)
    {
        proxy.handleException(ex);
    }
}

bool
AdminI::isServerEnabled(const ::std::string& id, const Ice::Current&) const
{
    ServerProxyWrapper proxy(_database, id);
    try
    {
        return proxy->isEnabled();
    }
    catch(const Ice::Exception& ex)
    {
        proxy.handleException(ex);
        return true; // Keeps the compiler happy.
    }
}

AdapterInfoSeq
AdminI::getAdapterInfo(const string& id, const Current&) const
{
    return _database->getAdapterInfo(id);
}

void
AdminI::removeAdapter(const string& adapterId, const Ice::Current&)
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
AdminI::addObject(const Ice::ObjectPrx& proxy, const ::Ice::Current& current)
{
    checkIsReadOnly();

    if(!proxy)
    {
        throw DeploymentException("proxy is null");
    }

    try
    {
        addObjectWithType(proxy, proxy->ice_id(), current);
    }
    catch(const Ice::LocalException& e)
    {
        ostringstream os;

        os << "failed to invoke ice_id() on proxy `" + current.adapter->getCommunicator()->proxyToString(proxy);
        os << "':\n" << e;
        throw DeploymentException(os.str());
    }
}

void
AdminI::updateObject(const Ice::ObjectPrx& proxy, const ::Ice::Current&)
{
    checkIsReadOnly();

    if(!proxy)
    {
        throw DeploymentException("proxy is null");
    }


    const Ice::Identity id = proxy->ice_getIdentity();
    if(id.category == _database->getInstanceName())
    {
        DeploymentException ex;
        ex.reason = "updating object `" + _database->getCommunicator()->identityToString(id) + "' is not allowed:\n";
        ex.reason += "objects with identity category `" + id.category + "' are managed by IceGrid";
        throw ex;
    }
    _database->updateObject(proxy);
}

void
AdminI::addObjectWithType(const Ice::ObjectPrx& proxy, const string& type, const ::Ice::Current&)
{
    checkIsReadOnly();

    if(!proxy)
    {
        throw DeploymentException("proxy is null");
    }

    const Ice::Identity id = proxy->ice_getIdentity();
    if(id.category == _database->getInstanceName())
    {
        DeploymentException ex;
        ex.reason = "adding object `" + _database->getCommunicator()->identityToString(id) + "' is not allowed:\n";
        ex.reason += "objects with identity category `" + id.category + "' are managed by IceGrid";
        throw ex;
    }

    ObjectInfo info;
    info.proxy = proxy;
    info.type = type;
    _database->addObject(info);
}

void
AdminI::removeObject(const Ice::Identity& id, const Ice::Current&)
{
    checkIsReadOnly();
    if(id.category == _database->getInstanceName())
    {
        DeploymentException ex;
        ex.reason = "removing object `" + _database->getCommunicator()->identityToString(id) + "' is not allowed:\n";
        ex.reason += "objects with identity category `" + id.category + "' are managed by IceGrid";
        throw ex;
    }
    _database->removeObject(id);
}

ObjectInfo
AdminI::getObjectInfo(const Ice::Identity& id, const Ice::Current&) const
{
    return _database->getObjectInfo(id);
}

ObjectInfoSeq
AdminI::getObjectInfosByType(const string& type, const Ice::Current&) const
{
    return _database->getObjectInfosByType(type);
}

ObjectInfoSeq
AdminI::getAllObjectInfos(const string& expression, const Ice::Current&) const
{
    return _database->getAllObjectInfos(expression);
}

NodeInfo
AdminI::getNodeInfo(const string& name, const Ice::Current&) const
{
    return toNodeInfo(_database->getNode(name)->getInfo());
}

ObjectPrx
AdminI::getNodeAdmin(const string& name, const Current& current) const
{
    //
    // Check if the node exists
    //
    _database->getNode(name);

    Ice::Identity adminId;
    adminId.name = name;
    adminId.category = _registry->getNodeAdminCategory();
    return current.adapter->createProxy(adminId);
}

bool
AdminI::pingNode(const string& name, const Current&) const
{
    try
    {
        _database->getNode(name)->getProxy()->ice_ping();
        return true;
    }
    catch(const NodeUnreachableException&)
    {
        return false;
    }
    catch(const Ice::ObjectNotExistException&)
    {
        throw NodeNotExistException();
    }
    catch(const Ice::LocalException&)
    {
        return false;
    }
}

LoadInfo
AdminI::getNodeLoad(const string& name, const Current&) const
{
    try
    {
        return _database->getNode(name)->getProxy()->getLoad();
    }
    catch(const Ice::ObjectNotExistException&)
    {
        throw NodeNotExistException();
    }
    catch(const Ice::LocalException& ex)
    {
        ostringstream os;
        os << ex;
        throw NodeUnreachableException(name, os.str());
    }
    return LoadInfo(); // Keep the compiler happy.
}

int
AdminI::getNodeProcessorSocketCount(const string& name, const Current&) const
{
    try
    {
        return _database->getNode(name)->getProxy()->getProcessorSocketCount();
    }
    catch(const Ice::OperationNotExistException&)
    {
        return 0; // Not supported.
    }
    catch(const Ice::ObjectNotExistException&)
    {
        throw NodeNotExistException(name);
        return 0;
    }
    catch(const Ice::LocalException& ex)
    {
        ostringstream os;
        os << ex;
        throw NodeUnreachableException(name, os.str());
        return 0;
    }
}

void
AdminI::shutdownNode(const string& name, const Current&)
{
    try
    {
        _database->getNode(name)->getProxy()->shutdown();
    }
    catch(const Ice::ObjectNotExistException&)
    {
        throw NodeNotExistException(name);
    }
    catch(const Ice::LocalException& ex)
    {
        ostringstream os;
        os << ex;
        throw NodeUnreachableException(name, os.str());
    }
}

string
AdminI::getNodeHostname(const string& name, const Current&) const
{
    try
    {
        return _database->getNode(name)->getInfo()->hostname;
    }
    catch(const Ice::ObjectNotExistException&)
    {
        throw NodeNotExistException(name);
    }
    catch(const Ice::LocalException& ex)
    {
        ostringstream os;
        os << ex;
        throw NodeUnreachableException(name, os.str());
        return ""; // Keep the compiler happy.
    }
}


StringSeq
AdminI::getAllNodeNames(const Current&) const
{
    return _database->getNodeCache().getAll("");
}

RegistryInfo
AdminI::getRegistryInfo(const string& name, const Ice::Current&) const
{
    if(name == _registry->getName())
    {
        return _registry->getInfo();
    }
    else
    {
        return toRegistryInfo(_database->getReplica(name)->getInfo());
    }
}

ObjectPrx
AdminI::getRegistryAdmin(const string& name, const Current& current) const
{
    if(name != _registry->getName())
    {
        //
        // Check if the replica exists
        //
        _database->getReplica(name);
    }

    Identity adminId;
    adminId.name = name;
    adminId.category = _registry->getReplicaAdminCategory();
    return current.adapter->createProxy(adminId);
}

bool
AdminI::pingRegistry(const string& name, const Current&) const
{
    if(name == _registry->getName())
    {
        return true;
    }

    try
    {
        _database->getReplica(name)->getProxy()->ice_ping();
        return true;
    }
    catch(const Ice::ObjectNotExistException&)
    {
        throw RegistryNotExistException();
    }
    catch(const Ice::LocalException&)
    {
        return false;
    }
    return false;
}

void
AdminI::shutdownRegistry(const string& name, const Current&)
{
    if(name == _registry->getName())
    {
        _registry->shutdown();
        return;
    }

    try
    {
        _database->getReplica(name)->getProxy()->shutdown();
    }
    catch(const Ice::ObjectNotExistException&)
    {
        throw RegistryNotExistException(name);
    }
    catch(const Ice::LocalException& ex)
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

SliceChecksumDict
AdminI::getSliceChecksums(const Current&) const
{
    return sliceChecksums();
}

void
AdminI::checkIsReadOnly() const
{
    if(_database->isReadOnly())
    {
        DeploymentException ex;
        ex.reason = "this operation is not allowed on a slave or read-only master registry.";
        throw ex;
    }
}

