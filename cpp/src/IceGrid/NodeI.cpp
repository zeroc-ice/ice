// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Ice/ProtocolPluginFacade.h> // Just to get the hostname
#include <IcePatch2/Util.h>
#include <IceGrid/NodeI.h>
#include <IceGrid/Activator.h>
#include <IceGrid/ServerI.h>
#include <IceGrid/ServerAdapterI.h>
#include <IceGrid/WaitQueue.h>
#include <IceGrid/TraceLevels.h>

using namespace std;
using namespace IcePatch2;
using namespace IceGrid;

NodeI::NodeI(const Ice::ObjectAdapterPtr& adapter,
	     const ActivatorPtr& activator, 
	     const WaitQueuePtr& waitQueue,
	     const TraceLevelsPtr& traceLevels,
	     const NodePrx& proxy,
	     const string& name) :
    _adapter(adapter),
    _activator(activator),
    _waitQueue(waitQueue),
    _traceLevels(traceLevels),
    _name(name),
    _hostname(IceInternal::getProtocolPluginFacade(adapter->getCommunicator())->getDefaultHost()),
    _proxy(proxy),
    _serial(1)
{
    const string dataDir = _adapter->getCommunicator()->getProperties()->getProperty("IceGrid.Node.Data");
    _serversDir = dataDir + (dataDir[dataDir.length() - 1] == '/' ? "" : "/") + "servers";
    _tmpDir = dataDir + (dataDir[dataDir.length() - 1] == '/' ? "" : "/") + "tmp";

    Ice::ObjectPrx registry = getCommunicator()->stringToProxy("IceGrid/Registry@IceGrid.Registry.Internal");
    _observer = RegistryPrx::uncheckedCast(registry)->getNodeObserver();
}

ServerPrx
NodeI::loadServer(const ServerDescriptorPtr& desc, StringAdapterPrxDict& adapters, const Ice::Current& current)
{
    Lock sync(*this);
    ++_serial;

    Ice::Identity id;
    id.category = "IceGridServer";
    id.name = desc->name;

    Ice::ObjectPtr servant = current.adapter->find(id);
    if(!servant)
    {
	servant = new ServerI(this, _serversDir, desc->name);
	current.adapter->add(servant, id);
    }

    ServerPrx proxy = ServerPrx::uncheckedCast(current.adapter->createProxy(id));
    proxy->load(desc, adapters);
    return proxy;
}

void
NodeI::destroyServer(const string& name, const Ice::Current& current)
{
    Lock sync(*this);
    ++_serial;

    Ice::Identity id;
    id.category = "IceGridServer";
    id.name = name;

    if(!current.adapter->find(id))
    {
	try
	{
	    removeRecursive(_serversDir + "/" + name);
	}
	catch(const string& msg)
	{
	    Ice::Warning out(_traceLevels->logger);
	    out << "removing server directory `" << _serversDir << "/" << name << "' failed:" << msg;
	}
    }
    else
    {
	ServerPrx proxy = ServerPrx::uncheckedCast(current.adapter->createProxy(id));
	try
	{
	    proxy->destroy();
	}
	catch(const Ice::LocalException&)
	{
	}
    }
}

std::string
NodeI::getName(const Ice::Current&) const
{
    return _name;
}

std::string
NodeI::getHostname(const Ice::Current&) const
{
    return _hostname;
}

void
NodeI::shutdown(const Ice::Current&) const
{
    _activator->shutdown();
}

Ice::CommunicatorPtr
NodeI::getCommunicator() const
{
    return _adapter->getCommunicator();
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

NodeObserverPrx
NodeI::getObserver() const
{
    return _observer;
}

NodeSessionPrx
NodeI::getSession() const
{
    Lock sync(_sessionMutex);
    return _session;
}

void
NodeI::setSession(const NodeSessionPrx& session)
{
    Lock sync(_sessionMutex);
    _session = session;
}

void
NodeI::keepAlive()
{
    NodeSessionPrx session = getSession();
    if(session)
    {
	try
	{
	    session->keepAlive();
	}
	catch(const Ice::LocalException&)
	{
	    setSession(0);
	}
    }
    else
    {
	try
	{
	    Ice::ObjectPrx registry = getCommunicator()->stringToProxy("IceGrid/Registry@IceGrid.Registry.Internal");
	    setSession(RegistryPrx::uncheckedCast(registry)->registerNode(_name, _proxy));
	    checkConsistency();
	}
	catch(const NodeActiveException& ex)
	{
	    _traceLevels->logger->error("a node with the same name is already registered and active");
	}
	catch(const Ice::LocalException& ex)
	{
	    _traceLevels->logger->warning("couldn't contact the IceGrid registry");
	}
    }
}

void
NodeI::stop()
{
    Lock sync(*this);
    if(_session)
    {
	try
	{
	    _session->destroy();
	}
	catch(const Ice::LocalException&)
	{
	}
    }
}

void
NodeI::checkConsistency()
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
    NodeSessionPrx session;
    do
    {
	{
	    Lock sync(*this);
	    if(serial == _serial)
	    {
		_serial = 1; // We can reset the serial number.
		checkConsistencyNoSync(servers);
		initObserver(servers);
		break;
	    }
	    serial = _serial;
	}
	session = getSession();
	servers = session ? session->getServers() : Ice::StringSeq();
    }
    while(session);
}

void
NodeI::checkConsistencyNoSync(const Ice::StringSeq& servers)
{
    //
    // Check if the servers directory doesn't contain more servers
    // than the registry really knows.
    //
    Ice::StringSeq contents = readDirectory(_serversDir);
    vector<string> remove;
    set_difference(contents.begin(), contents.end(), servers.begin(), servers.end(), back_inserter(remove));
		
    //
    // Remove the extra servers if possible.
    //
    {
	vector<string>::iterator p = remove.begin();
	while(p != remove.end())
	{
	    //
	    // If the server is loaded, we invoke on it to destroy it.
	    //
	    Ice::Identity id;
	    id.category = "IceGridServer";
	    id.name = *p;
	    if(_adapter->find(id))
	    {
		ServerPrx proxy = ServerPrx::uncheckedCast(_adapter->createProxy(id));
		try
		{
		    proxy->destroy();
		    p = remove.erase(p);
		    continue;
		}
		catch(const Ice::ObjectNotExistException&)
		{
		}
		catch(const Ice::LocalException& ex)
		{
		    Ice::Warning out(_traceLevels->logger);
		    out << "server `" << *p << "' destroy failed:" << ex;
		}
		catch(const string&)
		{
		    assert(false);
		}
	    }
	    
	    if(canRemoveServerDirectory(*p))
	    {
		//
		// If the server directory can be removed and we
		// either remove it or back it up before to remove it.
		//
		try
		{
		    removeRecursive(_serversDir + "/" + *p);
		}
		catch(const string& msg)
		{
		    Ice::Warning out(_traceLevels->logger);
		    out << "removing server directory `" << _serversDir << "/" << *p << "' failed:" << msg;
		}
		p = remove.erase(p);
	    }
	    else
	    {
		++p;
	    }
	}
    }
	
    if(remove.empty())
    {
	return;
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
    catch(const string&)
    {
	createDirectory(_tmpDir);
    }
    if(contents.size() < 10)
    {
	ostringstream os;
	os << "servers-" << contents.size();
	contents.push_back(os.str());
    }
    sort(contents.begin(), contents.end(), greater<string>());
    if(contents.size() == 10)
    {
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
	for(Ice::StringSeq::const_iterator p = contents.begin(); p != --contents.end(); ++p)
	{
	    rename(_tmpDir + "/" + *(p + 1), _tmpDir + "/" + *p);
	}
	createDirectoryRecursive(_tmpDir + "/servers-0");
	
	for(vector<string>::const_iterator p = remove.begin(); p != remove.end(); ++p)
	{
	    rename(_serversDir + "/" + *p, _tmpDir + "/servers-0/" + *p);
	}
    }
    catch(const string& msg)
    {
	Ice::Warning out(_traceLevels->logger);
	out << "rotation failed:" << msg;
    }
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
    if(!contents.empty())
    {
	return false;
    }

    contents = readDirectory(_serversDir + "/" + name + "/config");

    for(Ice::StringSeq::const_iterator p = contents.begin() ; p != contents.end(); ++p)
    {
	if(p->find("config") != 0)
	{
	    return false;
	}
    }

    contents = readDirectory(_serversDir + "/" + name + "/dbs");
    for(Ice::StringSeq::const_iterator p = contents.begin() ; p != contents.end(); ++p)
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
NodeI::initObserver(const Ice::StringSeq& servers)
{
    ServerDynamicInfoSeq serverInfos;
    AdapterDynamicInfoSeq adapterInfos;

    for(Ice::StringSeq::const_iterator p = servers.begin(); p != servers.end(); ++p)
    {
	Ice::Identity id;
	id.category = "IceGridServer";
	id.name = *p;
	if(_adapter->find(id))
	{
	    ServerPrx proxy = ServerPrx::uncheckedCast(_adapter->createProxy(id));
	    try
	    {
		ServerDynamicInfo server;
		server.name = *p;
		server.pid = proxy->getPid();
		server.state = proxy->getState();
		serverInfos.push_back(server);

		StringAdapterPrxDict adapters = proxy->getAdapters();
		for(StringAdapterPrxDict::const_iterator p = adapters.begin(); p != adapters.end(); ++p)
		{
		    AdapterDynamicInfo adapter;
		    adapter.id = p->first;
		    try
		    {
			adapter.proxy = p->second->getDirectProxy();
		    }
		    catch(const AdapterNotActiveException&)
		    {
		    }
		    catch(const Ice::ObjectNotExistException&)
		    {
		    }
		    adapterInfos.push_back(adapter);
		}
	    }
	    catch(const Ice::ObjectNotExistException&)
	    {
	    }
	}
    }

    try
    {
	_observer->init(_name, serverInfos, adapterInfos);
    }
    catch(const Ice::LocalException&)
    {
    }
}
