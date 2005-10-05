// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceGrid/AdminI.h>
#include <IceGrid/RegistryI.h>
#include <IceGrid/Database.h>
#include <IceGrid/Util.h>
#include <IceGrid/DescriptorHelper.h>
#include <Ice/LoggerUtil.h>
#include <Ice/TraceUtil.h>
#include <Ice/SliceChecksums.h>

using namespace std;
using namespace Ice;
using namespace IceGrid;

class ServerProxyWrapper
{
public:

    ServerProxyWrapper(const DatabasePtr& database, const string& id) : _id(id)
    {
	_proxy = database->getServerWithTimeouts(id, _activationTimeout, _deactivationTimeout, _node);
    }
    
    void
    useActivationTimeout()
    {
	_proxy = ServerPrx::uncheckedCast(_proxy->ice_timeout(_activationTimeout * 1000));
    }

    void
    useDeactivationTimeout()
    {
	_proxy = ServerPrx::uncheckedCast(_proxy->ice_timeout(_deactivationTimeout * 1000));
    }

    IceProxy::IceGrid::Server* 
    operator->() const
    {
	return _proxy.get();
    }

    void
    handleException(const Ice::Exception& ex)
    {
	try
	{
	    ex.ice_throw();
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

AdminI::AdminI(const CommunicatorPtr& communicator, const DatabasePtr& database, const RegistryPtr& registry) :
    _communicator(communicator),
    _database(database),
    _registry(registry)
{
}

AdminI::~AdminI()
{
}

void
AdminI::addApplication(const ApplicationDescriptor& descriptor, const Current&)
{
    _database->addApplicationDescriptor(0, descriptor);
}

void
AdminI::syncApplication(const ApplicationDescriptor& descriptor, const Current&)
{
    _database->syncApplicationDescriptor(0, descriptor);
}

void
AdminI::updateApplication(const ApplicationUpdateDescriptor& descriptor, const Current&)
{
    _database->updateApplicationDescriptor(0, descriptor);
}

void
AdminI::removeApplication(const string& name, const Current&)
{
    _database->removeApplicationDescriptor(0, name);
}

void
AdminI::patchApplication(const string& name, bool shutdown, const Current&)
{
    ApplicationHelper helper(_database->getApplicationDescriptor(name));    
    DistributionDescriptor appDistrib;
    map<string, DistributionDescriptorDict> nodeDistrib;
    helper.getDistributions(appDistrib, nodeDistrib);

    if(appDistrib.icepatch.empty() && nodeDistrib.empty())
    {
	return;
    }

    for(map<string, DistributionDescriptorDict>::const_iterator p = nodeDistrib.begin(); p != nodeDistrib.end(); ++p)
    {
	try
	{
	    _database->getNode(p->first)->patch(name, appDistrib, p->second, shutdown);
	}
	catch(const NodeNotExistException&)
	{
	}
	catch(const NodeUnreachableException&)
	{
	}
	catch(const Ice::ObjectNotExistException&)
	{
	}
	catch(const Ice::LocalException&)
	{
	}
    }
}

ApplicationDescriptor
AdminI::getApplicationDescriptor(const string& name, const Current&) const
{
    return _database->getApplicationDescriptor(name);
}

Ice::StringSeq
AdminI::getAllApplicationNames(const Current&) const
{
    return _database->getAllApplications();
}

ServerInfo
AdminI::getServerInfo(const string& id, const Current&) const
{
    return _database->getServerInfo(id, true);
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

bool
AdminI::startServer(const string& id, const Current&)
{
    ServerProxyWrapper proxy(_database, id);
    proxy.useActivationTimeout();
    try
    {
	return proxy->start();
    }
    catch(const Ice::TimeoutException&)
    {
	return false; // TODO: better exception?
    }
    catch(const Ice::Exception& ex)
    {
	proxy.handleException(ex);
    }
    return true;
}

void
AdminI::stopServer(const string& id, const Current&)
{
    ServerProxyWrapper proxy(_database, id);
    proxy.useDeactivationTimeout();
    try
    {
	proxy->stop();
    }
    catch(const Ice::TimeoutException&)
    {
    }
    catch(const Ice::Exception& ex)
    {
	proxy.handleException(ex);
    }
}

void
AdminI::patchServer(const string& id, bool shutdown, const Current&)
{
    ServerInfo info = _database->getServerInfo(id);
    ApplicationHelper helper(_database->getApplicationDescriptor(info.application));
    DistributionDescriptor appDistrib;
    map<string, DistributionDescriptorDict> nodeDistrib;
    helper.getDistributions(appDistrib, nodeDistrib, id);

    if(appDistrib.icepatch.empty() && nodeDistrib.empty())
    {
	return;
    }

    for(map<string, DistributionDescriptorDict>::const_iterator p = nodeDistrib.begin(); p != nodeDistrib.end(); ++p)
    {
	try
	{
	    _database->getNode(p->first)->patch(info.application, appDistrib, p->second, shutdown);
	}
	catch(const NodeNotExistException&)
	{
	}
	catch(const NodeUnreachableException&)
	{
	}
	catch(const Ice::ObjectNotExistException&)
	{
	}
	catch(const Ice::LocalException&)
	{
	}
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

void
AdminI::writeMessage(const string& id, const string& message, Int fd, const Current&)
{
    ServerProxyWrapper proxy(_database, id);
    try
    {
	proxy->writeMessage(message, fd);
    }
    catch(const Ice::Exception& ex)
    {
	proxy.handleException(ex);
    }
}


StringSeq
AdminI::getAllServerIds(const Current&) const
{
    return _database->getAllServers();
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

StringObjectProxyDict
AdminI::getAdapterEndpoints(const string& adapterId, const Current&) const
{
    int count;
    vector<pair<string, AdapterPrx> > adapters = _database->getAdapters(adapterId, count);
    StringObjectProxyDict adpts;
    for(vector<pair<string, AdapterPrx> >::const_iterator p = adapters.begin(); p != adapters.end(); ++p)
    {
	try
	{
	    adpts[p->first] = p->second->getDirectProxy();
	}
	catch(const Ice::ObjectNotExistException&)
	{
	}
	catch(const Ice::Exception&)
	{
	    adpts[p->first] = 0;
	}
    }
    return adpts;
}

void
AdminI::removeAdapterWithReplicaId(const string& adapterId, const string& replicaId, const Ice::Current&)
{
    _database->setAdapterDirectProxy(adapterId, replicaId, 0);
}

void
AdminI::removeAdapter(const string& adapterId, const Ice::Current&)
{
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
    try
    {
	addObjectWithType(proxy, proxy->ice_id(), current);
    }
    catch(const Ice::LocalException& e)
    {
	ostringstream os;
	os << "failed to invoke ice_id() on proxy `" + _communicator->proxyToString(proxy) + "':\n";
	os << e;

	DeploymentException ex;
	ex.reason =  os.str();
	throw ex;
    }
}

void 
AdminI::updateObject(const Ice::ObjectPrx& proxy, const ::Ice::Current& current)
{
    _database->updateObject(proxy);
}

void 
AdminI::addObjectWithType(const Ice::ObjectPrx& proxy, const string& type, const ::Ice::Current&)
{
    ObjectInfo info;
    info.proxy = proxy;
    info.type = type;
    _database->addObject(info);
}

void 
AdminI::removeObject(const Ice::Identity& id, const Ice::Current&)
{
    _database->removeObject(id);
}

ObjectInfo
AdminI::getObjectInfo(const Ice::Identity& id, const Ice::Current&) const
{
    return _database->getObjectInfo(id);
}

ObjectInfoSeq
AdminI::getAllObjectInfos(const string& expression, const Ice::Current&) const
{
    return _database->getAllObjectInfos(expression);
}

NodeInfo
AdminI::getNodeInfo(const string& name, const Ice::Current&) const
{
    return _database->getNodeInfo(name);
}

bool
AdminI::pingNode(const string& name, const Current&) const
{
    try
    {
	_database->getNode(name)->ice_ping();
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

void
AdminI::shutdownNode(const string& name, const Current&)
{
    NodePrx node = _database->getNode(name);
    try
    {
	node->shutdown();
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
    NodePrx node = _database->getNode(name);
    try
    {
	return node->getHostname();
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


StringSeq
AdminI::getAllNodeNames(const Current&) const
{
    return _database->getAllNodes();
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
