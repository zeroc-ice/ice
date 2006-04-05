// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IcePatch2/Util.h>
#include <IcePatch2/ClientUtil.h>
#include <IceGrid/NodeI.h>
#include <IceGrid/Activator.h>
#include <IceGrid/ServerI.h>
#include <IceGrid/ServerAdapterI.h>
#include <IceGrid/Util.h>
#include <IceGrid/WaitQueue.h>
#include <IceGrid/TraceLevels.h>

using namespace std;
using namespace IcePatch2;
using namespace IceGrid;

class LogPatcherFeedback : public PatcherFeedback
{
public:

    LogPatcherFeedback(const TraceLevelsPtr& traceLevels, const string& dest) : 
	_traceLevels(traceLevels),
	_startedPatch(false),
	_lastProgress(0),
	_dest(dest)
    {
    }

    void 
    setPatchingPath(const string& path)
    {
	_path = path;
	_startedPatch = false;
	_lastProgress = 0;
    }

    virtual bool
    noFileSummary(const string& reason)
    {
	if(_traceLevels->patch > 0)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->patchCat);
	    out << _dest << ": can't load summary file (will perform a thorough patch):\n" << reason;
	}
	return true;
    }

    virtual bool
    checksumStart()
    {
	if(_traceLevels->patch > 0)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->patchCat);
	    out << _dest << ": started checksum calculation";
	}
	return true;
    }

    virtual bool
    checksumProgress(const string& path)
    {
	if(_traceLevels->patch > 2)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->patchCat);
	    out << _dest << ": calculating checksum for " << getBasename(path);
	}
	return true;
    }

    virtual bool
    checksumEnd()
    {
	if(_traceLevels->patch > 0)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->patchCat);
	    out << _dest << ": finished checksum calculation";
	}
	return true;
    }

    virtual bool
    fileListStart()
    {
	if(_traceLevels->patch > 0)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->patchCat);
	    out << _dest << ": getting list of file to patch";
	}
	return true;
    }

    virtual bool
    fileListProgress(Ice::Int percent)
    {
	return true;
    }

    virtual bool
    fileListEnd()
    {
	if(_traceLevels->patch > 0)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->patchCat);
	    out << _dest << ": getting list of file to patch completed";
	}
	return true;
    }

    virtual bool
    patchStart(const string& path, Ice::Long size, Ice::Long totalProgress, Ice::Long totalSize)
    {
	if(_traceLevels->patch > 1 && totalSize > (1024 * 1024))
	{
	    int progress = static_cast<int>(static_cast<double>(totalProgress) / totalSize * 100.0);
	    progress /= 5;
	    progress *= 5;
	    if(progress != _lastProgress)
	    {
		_lastProgress = progress;
		Ice::Trace out(_traceLevels->logger, _traceLevels->patchCat);
		out << _dest << ": downloaded " << progress << "% (" << totalProgress << '/' << totalSize << ')';
		if(!_path.empty())
		{
		    out << " of " << _path;
		}
	    }
	}
	else if(_traceLevels->patch > 0)
	{
	    if(!_startedPatch)
	    {
		Ice::Trace out(_traceLevels->logger, _traceLevels->patchCat);
		int roundedSize = static_cast<int>(static_cast<double>(totalSize) / 1024);
		if(roundedSize == 0 && totalSize > 0)
		{
		    roundedSize = 1;
		}
		out << _dest << ": downloading " << (_path.empty() ? "" : (_path + " ")) << roundedSize << "KB ";
		_startedPatch = true;
	    }
	}
	
	return true;
    }

    virtual bool
    patchProgress(Ice::Long progress, Ice::Long size, Ice::Long totalProgress, Ice::Long totalSize)
    {
	return true;
    }

    virtual bool
    patchEnd()
    {	
	return true;
    }

private:

    const TraceLevelsPtr _traceLevels;
    bool _startedPatch;
    int _lastProgress;
    string _path;
    string _dest;
};

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
    _proxy(proxy),
    _waitTime(0),
    _serial(1),
    _platform(adapter->getCommunicator(), _traceLevels)
{
    _dataDir = _platform.getDataDir();
    _serversDir = _dataDir + "/servers";
    _tmpDir = _dataDir + "/tmp";

    Ice::PropertiesPtr properties = getCommunicator()->getProperties();
    const string instanceNameProperty = "IceGrid.InstanceName";
    const_cast<string&>(_instName) = properties->getPropertyWithDefault(instanceNameProperty, "IceGrid");
    const_cast<Ice::Int&>(_waitTime) = properties->getPropertyAsIntWithDefault("IceGrid.Node.WaitTime", 60);
}

NodeI::~NodeI()
{
}

void
NodeI::loadServer_async(const AMD_Node_loadServerPtr& amdCB,
			const string& application,
			const ServerDescriptorPtr& desc,
			const Ice::Current& current)
{
    Lock sync(*this);
    ++_serial;

    Ice::Identity id = createServerIdentity(desc->id);

    //
    // Check if we already have a servant for this server. If that's
    // the case, the server is already loaded and we just need to
    // update it.
    //
    ServerIPtr server = ServerIPtr::dynamicCast(_adapter->find(id));
    ServerPrx proxy = ServerPrx::uncheckedCast(_adapter->createProxy(id));
    if(!server)
    {
	server = new ServerI(this, proxy, _serversDir, desc->id, _waitTime);
	_adapter->add(server, id);
    }
    else
    {
	if(server->getDescriptor()->applicationDistrib)
	{
	    removeServer(server);
	}
    }

    if(desc->applicationDistrib)
    {
	addServer(application, server);
    }

    //
    // Update the server with the new descriptor information.
    //
    server->load(amdCB, application, desc);
}

void
NodeI::destroyServer_async(const AMD_Node_destroyServerPtr& amdCB, const string& name, const Ice::Current& current)
{
    Lock sync(*this);
    ++_serial;

    Ice::Identity id = createServerIdentity(name);
    ServerIPtr server = ServerIPtr::dynamicCast(_adapter->find(id));
    if(server)
    {
	//
	// Destroy the server object if it's loaded.
	//
	removeServer(server);
	server->destroy(amdCB);
    }
    else
    {
	//
	// Delete the server directory from the disk.
	//
	try
	{
	    removeRecursive(_serversDir + "/" + name);
	}
	catch(const string&)
	{
	}
	amdCB->ice_response();
    }
}

void
NodeI::patch(const string& application, 
	     const string& server,
	     const DistributionDescriptor& appDistrib,
	     bool shutdown, 
	     const Ice::Current&)
{
    set<ServerIPtr> servers;
    {
	Lock sync(*this);

	while(_patchInProgress.find(application) != _patchInProgress.end())
	{
	    wait();
	}
	_patchInProgress.insert(application);

	if(!appDistrib.icepatch.empty())
	{
	    //
	    // Get all the application servers (even the ones which
	    // don't have a distribution since they depend on the
	    // application distribution).
	    //
	    servers = getApplicationServers(application);
	}
	else if(server.empty())
	{
	    //
	    // Get all the application servers which have a distribution.
	    //
	    servers = getApplicationServers(application);
	    set<ServerIPtr>::iterator s = servers.begin();
	    while(s != servers.end())
	    {
		if((*s)->getDistribution().icepatch.empty())
		{
		    servers.erase(s++);
		}
		else
		{
		    ++s;
		}
	    }
	}
	else
	{
	    //
	    // Get the given server.
	    //
	    Ice::Identity id = createServerIdentity(server);
	    ServerIPtr svr = ServerIPtr::dynamicCast(_adapter->find(id));
	    if(svr)
	    {
		servers.insert(svr);
	    }
	}
    }

    try
    {
	set<ServerIPtr>::iterator s = servers.begin(); 
	vector<string> running;
	while(s != servers.end())
	{
	    if(!(*s)->startPatch(shutdown))
	    {
		running.push_back((*s)->getId());
		servers.erase(s++);
	    }
	    else
	    {
		++s;
	    }
	}

	if((servers.empty() || !appDistrib.icepatch.empty()) && !running.empty())
	{
	    PatchException ex;
	    ex.reason = "patch on node `" + _name + "' failed:\n";
	    if(running.size() == 1)
	    {
		ex.reason += "server `" + toString(running) + "' is active";
	    }
	    else
	    {
		ex.reason += "servers `" + toString(running, ", ") + "' are active";
	    }
	    throw ex;
	}

	for(s = servers.begin(); s != servers.end(); ++s)
	{
	    (*s)->waitForPatch();
	}

	try
	{
	    // 
	    // Patch the application.
	    //
	    FileServerPrx icepatch;
	    if(!appDistrib.icepatch.empty())
	    {
		icepatch = FileServerPrx::checkedCast(getCommunicator()->stringToProxy(appDistrib.icepatch));
		if(!icepatch)
		{
		    throw "proxy `" + appDistrib.icepatch + "' is not a file server.";
		}
		patch(icepatch, "distrib/" + application, appDistrib.directories);
	    }

	    //
	    // Patch the server(s).
	    //
	    for(s = servers.begin(); s != servers.end(); ++s)
	    {
		DistributionDescriptor dist = (*s)->getDistribution();
		if(dist.icepatch.empty() || (!server.empty() && (*s)->getId() != server))
		{
		    continue;
		}

		icepatch = FileServerPrx::checkedCast(getCommunicator()->stringToProxy(dist.icepatch));
		if(!icepatch)
		{
		    throw "proxy `" + dist.icepatch + "' is not a file server.";
		}
		patch(icepatch, "servers/" + (*s)->getId() + "/distrib", dist.directories);

		if(!server.empty())
		{
		    break;
		}
	    }
	}
	catch(const Ice::LocalException& ex)
	{
	    ostringstream os;
	    os << "patch on node `" + _name + "' failed:\n";
	    os << ex;
	    throw PatchException(os.str());
	}
	catch(const string& ex)
	{
	    string msg = "patch on node `" + _name + "' failed:\n" + ex;
	    throw PatchException(msg);
	}

	for(s = servers.begin(); s != servers.end(); ++s)
	{
	    (*s)->finishPatch();
	}
	{
	    Lock sync(*this);
	    _patchInProgress.erase(application);
	    notifyAll();
	}
    }
    catch(...)
    {
	for(set<ServerIPtr>::const_iterator s = servers.begin(); s != servers.end(); ++s)
	{
	    (*s)->finishPatch();
	}
	{
	    Lock sync(*this);
	    _patchInProgress.erase(application);
	    notifyAll();
	}
	throw;
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
    return _platform.getHostname();
}

LoadInfo
NodeI::getLoad(const Ice::Current&) const
{
    return _platform.getLoadInfo();
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

Ice::ObjectAdapterPtr
NodeI::getAdapter() const
{
    return _adapter;
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
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_sessionMonitor);
    return _observer;
}

NodeSessionPrx
NodeI::getSession() const
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_sessionMonitor);
    return _session;
}

void
NodeI::setSession(const NodeSessionPrx& session, const NodeObserverPrx& observer)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_sessionMonitor);
    _session = session;
    _observer = observer;
    if(_session)
    {
	_sessionMonitor.notifyAll();
    }
}


int
NodeI::keepAlive()
{
    NodeSessionPrx session = getSession();
    if(session)
    {
	try
	{
	    session->keepAlive(_platform.getLoadInfo());
	}
	catch(const Ice::LocalException&)
	{
	    setSession(0, 0);
	}
    }
    else
    {
	try
	{
	    Ice::ObjectPrx obj = getCommunicator()->stringToProxy(_instName + "/Registry");
	    RegistryPrx registry = RegistryPrx::uncheckedCast(obj);
	    NodeObserverPrx observer;
	    setSession(registry->registerNode(_name, _proxy, _platform.getNodeInfo(), observer), observer);
	    checkConsistency();
	    return registry->getTimeout() / 2;
	}
	catch(const NodeActiveException&)
	{
	    _traceLevels->logger->error("a node with the same name is already registered and active");
	}
	catch(const Ice::LocalException& ex)
	{
	    ostringstream os;
	    os << "couldn't contact the IceGrid registry:\n" << ex;
	    _traceLevels->logger->warning(os.str());
	}
    }
    return 0;
}

void
NodeI::waitForSession()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_sessionMonitor);
    while(!_session)
    {
	_sessionMonitor.timedWait(IceUtil::Time::seconds(_waitTime));
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
	    _session = 0;
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
	sort(servers.begin(), servers.end());
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
	    ServerIPtr server = ServerIPtr::dynamicCast(_adapter->find(createServerIdentity(*p)));
	    if(server)
	    {
		//
		// If the server is loaded, we invoke on it to destroy it.
		//
		try
		{
		    server->destroy(0);
		    p = remove.erase(p);
		    continue;
		}
		catch(const Ice::LocalException& ex)
		{
		    Ice::Error out(_traceLevels->logger);
		    out << "server `" << *p << "' destroy failed:" << ex;
		}
		catch(const string&)
		{
		    assert(false);
		}
	    }
	    
	    try
	    {
		if(canRemoveServerDirectory(*p))
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
	    catch(const string& msg)
	    {
		Ice::Warning out(_traceLevels->logger);
		out << "removing server directory `" << _serversDir << "/" << *p << "' failed:" << msg;
	    }

	    ++p;
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
	sort(contents.begin(), contents.end(), greater<string>());
    }
    else if(contents.size() == 10)
    {
	sort(contents.begin(), contents.end(), greater<string>());
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
	Ice::StringSeq::const_iterator p;
	for(p = contents.begin(); p != (contents.end() - 1); ++p)
	{
	    rename(_tmpDir + "/" + *(p + 1), _tmpDir + "/" + *p);
	}
	createDirectoryRecursive(_tmpDir + "/servers-0");
	for(p = remove.begin(); p != remove.end(); ++p)
	{
	    rename(_serversDir + "/" + *p, _tmpDir + "/servers-0/" + *p);
	}
    }
    catch(const string& msg)
    {
	Ice::Warning out(_traceLevels->logger);
	out << "rotation failed: " << msg;
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
    remove(contents.begin(), contents.end(), "distrib");
    if(!contents.empty())
    {
	return false;
    }
    
    contents = readDirectory(_serversDir + "/" + name + "/config");
    
    Ice::StringSeq::const_iterator p;
    for(p = contents.begin() ; p != contents.end(); ++p)
    {
	if(p->find("config") != 0)
	{
	    return false;
	}
    }
    
    contents = readDirectory(_serversDir + "/" + name + "/dbs");
    for(p = contents.begin() ; p != contents.end(); ++p)
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
	Ice::Identity id = createServerIdentity(*p);
	ServerIPtr server = ServerIPtr::dynamicCast(_adapter->find(id));
	if(server)
	{
	    try
	    {
		server->getDynamicInfo(serverInfos, adapterInfos);
	    }
	    catch(const Ice::ObjectNotExistException&)
	    {
	    }
	}
    }

    try
    {
	NodeDynamicInfo info;
	info.name = _name;
	info.info = _platform.getNodeInfo();
	info.servers = serverInfos;
	info.adapters = adapterInfos;
	_observer->nodeUp(info);
    }
    catch(const Ice::LocalException& ex)
    {
	Ice::Warning out(_traceLevels->logger);
	out << "unexpected observer exception:\n" << ex;
    }
}

void
NodeI::patch(const FileServerPrx& icepatch, const string& destination, const vector<string>& directories)
{
    PatcherFeedbackPtr feedback = new LogPatcherFeedback(_traceLevels, destination);
    IcePatch2::createDirectory(_dataDir + "/" + destination);
    PatcherPtr patcher = new Patcher(icepatch, feedback, _dataDir + "/" + destination, false, 100, 1);
    bool aborted = !patcher->prepare();
    if(!aborted)
    {
	if(directories.empty())
	{
	    aborted = !patcher->patch("");
	}
	else
	{
	    for(vector<string>::const_iterator p = directories.begin(); p != directories.end(); ++p)
	    {
		dynamic_cast<LogPatcherFeedback*>(feedback.get())->setPatchingPath(*p);
		if(!patcher->patch(*p))
		{
		    aborted = true;
		    break;
		}
	    }
	}
    }
    if(!aborted)
    {
	patcher->finish();
    }
}

void
NodeI::addServer(const string& application, const ServerIPtr& server)
{
    map<string, set<ServerIPtr> >::iterator p = _serversByApplication.find(application);
    if(p == _serversByApplication.end())
    {
	map<string, set<ServerIPtr> >::value_type v(application, set<ServerIPtr>());
	p = _serversByApplication.insert(p, v);
    }
    p->second.insert(server);
}

void
NodeI::removeServer(const ServerIPtr& server)
{
    map<string, set<ServerIPtr> >::iterator p = _serversByApplication.find(server->getApplication());
    if(p != _serversByApplication.end())
    {
	p->second.erase(server);
	if(p->second.empty())
	{
	    _serversByApplication.erase(p);
	}
    }
}

set<ServerIPtr>
NodeI::getApplicationServers(const string& application)
{
    set<ServerIPtr> servers;
    map<string, set<ServerIPtr> >::const_iterator p = _serversByApplication.find(application);
    if(p != _serversByApplication.end())
    {
	servers = p->second;
    }
    return servers;
}

Ice::Identity
NodeI::createServerIdentity(const string& name)
{
    Ice::Identity id;
    id.category = _instName + "-Server";
    id.name = name;
    return id;
}
