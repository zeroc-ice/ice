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
#include <IceGrid/DescriptorHelper.h>
#include <Ice/LoggerUtil.h>
#include <Ice/TraceUtil.h>
#include <Ice/SliceChecksums.h>

using namespace std;
using namespace Ice;
using namespace IceGrid;

AdminI::AdminI(const CommunicatorPtr& communicator, 
	       const DatabasePtr& database,
	       const RegistryPtr& registry) :
    _communicator(communicator),
    _database(database),
    _registry(registry)
{
}

AdminI::~AdminI()
{
}

void
AdminI::addApplication(const ApplicationDescriptorPtr& descriptor, const Current&)
{
    _database->addApplicationDescriptor(descriptor);
}

void
AdminI::updateApplication(const ApplicationDescriptorPtr& descriptor, const Current&)
{
    _database->updateApplicationDescriptor(descriptor);
}

void
AdminI::removeApplication(const string& name, const Current&)
{
    _database->removeApplicationDescriptor(name);
}

ApplicationDescriptorPtr
AdminI::getApplicationDescriptor(const string& name, const Current&) const
{
    return _database->getApplicationDescriptor(name);
}

void
AdminI::instantiateApplicationServer(const string& name, const string& tmpl, const StringStringDict& variables,
				     const Current&)
{
    ApplicationDescriptorHelper helper(_communicator, _database->getApplicationDescriptor(name));
    helper.addServer(tmpl, variables);
    _database->updateApplicationDescriptor(helper.getDescriptor());
}

Ice::StringSeq
AdminI::getAllApplicationNames(const Current&) const
{
    return _database->getAllApplications();
}

void
AdminI::addServer(const ApplicationDescriptorPtr& server, const Current&)
{
//     if(server->application.empty())
//     {
//  	ApplicationDescriptorPtr application = new ApplicationDescriptor();
//  	application->name = '_' + server->name;
// 	ApplicationDescriptorHelper helper(_communicator, application);
// 	helper.addServerTemplate("_" + server->name, server);
// 	helper.addServer("_" + server->name, variables);

// 	application->servers.push_back(server);
// 	try
// 	{
// 	    _database->addApplicationDescriptor(application);
// 	}
// 	catch(const ApplicationExistsException&)
// 	{
// 	    ServerExistsException ex;
// 	    ex.name = server->name;
// 	    throw ex;
// 	}
//     }
//     else
//     {
// 	try
// 	{
// 	    ApplicationDescriptorPtr application = _database->getApplicationDescriptor(server->application);
// 	    application->servers.push_back(server);
// 	    _database->updateApplicationDescriptor(application);
// 	}
// 	catch(const ApplicationNotExistException&)
// 	{
// 	    DeploymentException ex;
// 	    ex.reason = "application `" + server->application + "' doesn't exist";
// 	    throw ex;
// 	}
//     }
}

void
AdminI::updateServer(const ApplicationDescriptorPtr& desc, const Current&)
{
    //
    // TODO
    //
//     ServerDescriptorPtr server = _database->getServerDescriptor(newServer->name);
//     try
//     {
// 	if(server->application.empty())
// 	{
// 	    ApplicationDescriptorPtr application = new ApplicationDescriptor();
// 	    application->name = '_' + newServer->name;
// 	    application->servers.push_back(newServer);
// 	    _database->updateApplicationDescriptor(application);
// 	}
// 	else
// 	{
// 	    ApplicationDescriptorPtr application = _database->getApplicationDescriptor(server->application);
// 	    for(ServerDescriptorSeq::iterator p = application->servers.begin(); p != application->servers.end(); ++p)
// 	    {
// 		application->servers.erase(p);
// 		application->servers.push_back(newServer);
// 	    }
// 	    _database->updateApplicationDescriptor(application);
// 	}
//     }
//     catch(const ApplicationNotExistException&)
//     {
// 	ServerNotExistException ex;
// 	ex.name = newServer->name;
// 	throw ex;
//     }

}

void
AdminI::removeServer(const string& name, const Current&)
{
    //
    // TODO
    //
//     ServerDescriptorPtr server = _database->getServerDescriptor(name);
//     try
//     {
// 	if(server->application.empty())
// 	{
// 	    _database->removeApplicationDescriptor('_' + name);
// 	}
// 	else
// 	{
// 	    ApplicationDescriptorPtr application = _database->getApplicationDescriptor(server->application);
// 	    for(ServerDescriptorSeq::iterator p = application->servers.begin(); p != application->servers.end(); ++p)
// 	    {
// 		if((*p)->name == name)
// 		{
// 		    application->servers.erase(p);
// 		    break;
// 		}
// 	    }
// 	    _database->updateApplicationDescriptor(application);
// 	}
//     }
//     catch(const ApplicationNotExistException&)
//     {
// 	ServerNotExistException ex;
// 	ex.name = name;
// 	throw ex;
//     }
}

InstanceDescriptor
AdminI::getServerDescriptor(const string& name, const Current&) const
{
    return _database->getServerDescriptor(name);
}

ServerState
AdminI::getServerState(const string& name, const Current&) const
{
    ServerPrx server = _database->getServer(name);
    try
    {
	return server->getState();
    }
    catch(const Ice::ObjectNotExistException&)
    {
	throw ServerNotExistException();
    }
    catch(const Ice::LocalException& ex)
    {
	cerr << ex << endl;
	throw NodeUnreachableException();
    }
}

Ice::Int
AdminI::getServerPid(const string& name, const Current&) const
{
    ServerPrx server = _database->getServer(name);
    try
    {
	return server->getPid();
    }
    catch(const Ice::ObjectNotExistException& ex)
    {
	cerr << ex << endl;
	throw ServerNotExistException();
    }
    catch(const Ice::LocalException& ex)
    {
	cerr << ex << endl;
	throw NodeUnreachableException();
    }
}

bool
AdminI::startServer(const string& name, const Current&)
{
    ServerPrx server = _database->getServer(name);
    try
    {
	return server->start(Manual);
    }
    catch(const Ice::ObjectNotExistException&)
    {
	throw ServerNotExistException();
    }
    catch(const Ice::LocalException&)
    {
	throw NodeUnreachableException();
    }
}

void
AdminI::stopServer(const string& name, const Current&)
{
    ServerPrx server = _database->getServer(name);
    try
    {
	server->stop();
    }
    catch(const Ice::ObjectNotExistException&)
    {
	throw ServerNotExistException();
    }
    catch(const Ice::LocalException&)
    {
	throw NodeUnreachableException();
    }
}

void
AdminI::sendSignal(const string& name, const string& signal, const Current&)
{
    ServerPrx server = _database->getServer(name);
    try
    {
	server->sendSignal(signal);
    }
    catch(const Ice::ObjectNotExistException&)
    {
	throw ServerNotExistException();
    }
    catch(const Ice::LocalException&)
    {
	throw NodeUnreachableException();
    }
}

void
AdminI::writeMessage(const string& name, const string& message, Int fd, const Current&)
{
    ServerPrx server = _database->getServer(name);
    try
    {
	server->writeMessage(message, fd);
    }
    catch(const Ice::ObjectNotExistException&)
    {
	throw ServerNotExistException();
    }
    catch(const Ice::LocalException&)
    {
	throw NodeUnreachableException();
    }
}


StringSeq
AdminI::getAllServerNames(const Current&) const
{
    return _database->getAllServers();
}

ServerActivation 
AdminI::getServerActivation(const ::std::string& name, const Ice::Current&) const
{
    ServerPrx server = _database->getServer(name);
    try
    {
	return server->getActivationMode();
    }
    catch(const Ice::ObjectNotExistException&)
    {
	throw ServerNotExistException();
    }
    catch(const Ice::LocalException&)
    {
	throw NodeUnreachableException();
    }
}

void 
AdminI::setServerActivation(const ::std::string& name, ServerActivation mode, const Ice::Current&)
{
    ServerPrx server = _database->getServer(name);
    try
    {
	server->setActivationMode(mode);
    }
    catch(const Ice::ObjectNotExistException&)
    {
	throw ServerNotExistException();
    }
    catch(const Ice::LocalException&)
    {
	throw NodeUnreachableException();
    }
}

string 
AdminI::getAdapterEndpoints(const string& id, const Current&) const
{
    AdapterPrx adapter = _database->getAdapter(id);
    try
    {
	return _communicator->proxyToString(adapter->getDirectProxy());
    }
    catch(AdapterNotActiveException&)
    {
	return "";
    }
    catch(const Ice::ObjectNotExistException&)
    {
	throw AdapterNotExistException();
    }
    catch(const Ice::LocalException&)
    {
	throw NodeUnreachableException();
    }
}

StringSeq
AdminI::getAllAdapterIds(const Current&) const
{
    return _database->getAllAdapters();
}

void 
AdminI::addObject(const Ice::ObjectPrx& proxy, const ::Ice::Current& current)
{
    ObjectDescriptor desc;
    desc.proxy = proxy;
    
    try
    {
	addObjectWithType(proxy, proxy->ice_id(), current);
    }
    catch(const Ice::LocalException&)
    {
	DeploymentException ex;
	ex.reason = "Couldn't invoke on the object to get its interface.";
	throw ex;
    }
}

void 
AdminI::updateObject(const Ice::ObjectPrx& proxy, const ::Ice::Current& current)
{
    _database->updateObjectDescriptor(proxy);
}

void 
AdminI::addObjectWithType(const Ice::ObjectPrx& proxy, const string& type, const ::Ice::Current&)
{
    ObjectDescriptor desc;
    desc.proxy = proxy;
    desc.type = type;
    _database->addObjectDescriptor(desc);
}

void 
AdminI::removeObject(const Ice::Identity& id, const Ice::Current&)
{
    _database->removeObjectDescriptor(id);
}

ObjectDescriptor
AdminI::getObjectDescriptor(const Ice::Identity& id, const Ice::Current&) const
{
    return _database->getObjectDescriptor(id);
}

ObjectDescriptorSeq
AdminI::getAllObjectDescriptors(const string& expression, const Ice::Current&) const
{
    return _database->getAllObjectDescriptors(expression);
}

bool
AdminI::pingNode(const string& name, const Current&) const
{
    NodePrx node = _database->getNode(name);
    try
    {
	node->ice_ping();
	return true;
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
	throw NodeNotExistException();
    }
    catch(const Ice::LocalException&)
    {
	throw NodeUnreachableException();
    }
}

void
AdminI::removeNode(const string& name, const Current&)
{
    //
    // Remove the node.
    //
    _database->removeNode(name);
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
	throw NodeNotExistException();
    }
    catch(const Ice::LocalException&)
    {
	throw NodeUnreachableException();
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
