// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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

#if defined(_WIN32)
#   include <direct.h> // For _getcwd
#endif

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
    _waitTime(adapter->getCommunicator()->getProperties()->getPropertyAsIntWithDefault("IceGrid.Node.WaitTime", 60)),
    _serial(1),
    _platform(adapter->getCommunicator(), _traceLevels)
{
    _dataDir = _adapter->getCommunicator()->getProperties()->getProperty("IceGrid.Node.Data");
    if(!isAbsolute(_dataDir))
    {
#ifdef _WIN32
	char cwd[_MAX_PATH];
	if(_getcwd(cwd, _MAX_PATH) == NULL)
#else
	char cwd[PATH_MAX];
	if(getcwd(cwd, PATH_MAX) == NULL)
#endif
	{
	    throw "cannot get the current directory:\n" + lastError();
	}
	
	_dataDir = string(cwd) + '/' + _dataDir;
    }
    if(_dataDir[_dataDir.length() - 1] == '/')
    {
	_dataDir = _dataDir.substr(0, _dataDir.length() - 1);
    }

    _sharedDir = _dataDir + "/shared";
    _serversDir = _dataDir + "/servers";
    _tmpDir = _dataDir + "/tmp";
}

NodeI::~NodeI()
{
}

ServerPrx
NodeI::loadServer(const string& application,
		  const ServerDescriptorPtr& desc, 
		  AdapterPrxDict& adapters, 
		  int& activationTimeout, 
		  int& deactivationTimeout, 
		  const Ice::Current& current)
{
    Lock sync(*this);
    ++_serial;

    Ice::Identity id;
    id.category = "IceGridServer";
    id.name = desc->id;

    Ice::ObjectPtr servant = current.adapter->find(id);
    ServerPrx proxy = ServerPrx::uncheckedCast(current.adapter->createProxy(id));
    bool init = !servant;
    if(!servant)
    {
	servant = new ServerI(this, proxy, _serversDir, desc->id, _waitTime);
	current.adapter->add(servant, id);
    }
    proxy->update(application, desc, init, adapters, activationTimeout, deactivationTimeout);
    
    map<string, set<ServerIPtr> >::iterator p = _serversByApplication.find(application);
    if(p == _serversByApplication.end())
    {
	p = _serversByApplication.insert(p, make_pair(application, set<ServerIPtr>()));
    }
    p->second.insert(ServerIPtr::dynamicCast(servant));
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

    Ice::ObjectPtr servant = current.adapter->find(id);
    if(!servant)
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
	    ServerIPtr server = ServerIPtr::dynamicCast(servant);
	    ServerDescriptorPtr desc = server->getDescriptor();
	    proxy->destroy();

	    map<string, set<ServerIPtr> >::iterator p = _serversByApplication.find(server->getApplication());
	    assert(p != _serversByApplication.end());
	    p->second.erase(server);
	    if(p->second.empty())
	    {
		_serversByApplication.erase(p);
	    }
	}
	catch(const Ice::LocalException&)
	{
	}
    }
}

void
NodeI::patch(const string& application, 
	     const DistributionDescriptor& appDistrib,
	     const DistributionDescriptorDict& serverDistribs, 
	     bool shutdown, 
	     const Ice::Current&)
{
    Lock sync(*this);

    set<ServerIPtr> servers;
    {
	map<string, set<ServerIPtr> >::const_iterator p = _serversByApplication.find(application);
	if(p != _serversByApplication.end())
	{
	    servers = p->second;
	}
    }

    try
    {
	vector<string> running;
	set<ServerIPtr>::const_iterator p;
	for(p = servers.begin(); p != servers.end(); ++p)
	{
	    if(!(*p)->startUpdating(shutdown))
	    {
		running.push_back((*p)->getId());
	    }
	}

	if(!running.empty())
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

	try
	{
	    // 
	    // Patch the application.
	    //
	    FileServerPrx icepatch = FileServerPrx::checkedCast(getCommunicator()->stringToProxy(appDistrib.icepatch));
	    if(!icepatch)
	    {
		throw "proxy `" + appDistrib.icepatch + "' is not a file server.";
	    }
	    patch(icepatch, "shared/" + application, appDistrib.directories);

	    //
	    // Patch the servers.
	    //
	    for(DistributionDescriptorDict::const_iterator p = serverDistribs.begin(); p != serverDistribs.end(); ++p)
	    {
		icepatch = FileServerPrx::checkedCast(getCommunicator()->stringToProxy(p->second.icepatch));
		if(!icepatch)
		{
		    throw "proxy `" + p->second.icepatch + "' is not a file server.";
		}
		patch(icepatch, "servers/" + p->first + "/distribution", p->second.directories);
	    }
	}
	catch(const Ice::LocalException& ex)
	{
	    ostringstream os;
	    os << "patch on node `" + _name + "' failed:\n";
	    os << ex;
	    _traceLevels->logger->warning(os.str());
	    throw PatchException(os.str());
	}
	catch(const string& ex)
	{
	    string msg = "patch on node `" + _name + "' failed:\n" + ex;
	    _traceLevels->logger->error(msg);
	    throw PatchException(msg);
	}

	for(p = servers.begin(); p != servers.end(); ++p)
	{
	    (*p)->finishUpdating();
	}
    }
    catch(...)
    {
	for(set<ServerIPtr>::const_iterator p = servers.begin(); p != servers.end(); ++p)
	{
	    (*p)->finishUpdating();
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
    Lock sync(_sessionMutex);
    return _observer;
}

NodeSessionPrx
NodeI::getSession() const
{
    Lock sync(_sessionMutex);
    return _session;
}

void
NodeI::setSession(const NodeSessionPrx& session, const NodeObserverPrx& observer)
{
    Lock sync(_sessionMutex);
    _session = session;
    _observer = observer;
}

void
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
	    Ice::ObjectPrx object = getCommunicator()->stringToProxy("IceGrid/Registry@IceGrid.Registry.Internal");
	    RegistryPrx registry = RegistryPrx::uncheckedCast(object);
	    NodeObserverPrx observer;
	    setSession(registry->registerNode(_name, _proxy, _platform.getNodeInfo(), observer), observer);
	    checkConsistency();
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
	Ice::Identity id;
	id.category = "IceGridServer";
	id.name = *p;
	ServerIPtr server = ServerIPtr::dynamicCast(_adapter->find(id));
	if(server)
	{
	    try
	    {
		server->addDynamicInfo(serverInfos, adapterInfos);
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
	info.info.dataDir = _dataDir;
	info.servers = serverInfos;
	info.adapters = adapterInfos;
	_observer->nodeUp(info);
    }
    catch(const Ice::LocalException&)
    {
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

