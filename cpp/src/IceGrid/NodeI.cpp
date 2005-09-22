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
#elif defined(__APPLE__)
#   include <sys/sysctl.h>
#elif defined(__hpux)
#   include <sys/pstat.h>
#elif defined(_AIX)
#   include <nlist.h>
#   include <fcntl.h>
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
    _hostname(IceInternal::getProtocolPluginFacade(adapter->getCommunicator())->getDefaultHost()),
    _proxy(proxy),
    _waitTime(adapter->getCommunicator()->getProperties()->getPropertyAsIntWithDefault("IceGrid.Node.WaitTime", 60)),
    _serial(1)
{
    string dataDir = _adapter->getCommunicator()->getProperties()->getProperty("IceGrid.Node.Data");
    if(!isAbsolute(dataDir))
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
	
	dataDir = string(cwd) + '/' + dataDir;
    }

    _serversDir = dataDir + (dataDir[dataDir.length() - 1] == '/' ? "" : "/") + "servers";
    _tmpDir = dataDir + (dataDir[dataDir.length() - 1] == '/' ? "" : "/") + "tmp";

#if defined(_WIN32)
    PDH_STATUS err = PdhOpenQuery(0, 0, &_query);
    if(err != ERROR_SUCCESS)
    {
        Ice::SyscallException ex(__FILE__, __LINE__);
        ex.error = err;
	Ice::Warning out(_traceLevels->logger);
	out << "can't open performance data query:\n" << ex;
    }
    err = PdhAddCounter(_query, "\\Processor(_Total)\\% Processor Time", 0, &_counter);
    if(err != ERROR_SUCCESS)
    {
        Ice::SyscallException ex(__FILE__, __LINE__);
        ex.error = err;
	Ice::Warning out(_traceLevels->logger);
	out << "can't add performance counter:\n" << ex;
    }
    _usages1.insert(_usages1.end(), 1 * 60 / 5, 0); // 1 sample every 5 seconds during 1 minutes.
    _usages5.insert(_usages5.end(), 5 * 60 / 5, 0); // 1 sample every 5 seconds during 5 minutes.
    _usages15.insert(_usages15.end(), 15 * 60 / 5, 0); // 1 sample every 5 seconds during 15 minutes.
    _last1Total = 0;
    _last5Total = 0;
    _last15Total = 0;
#else
#if defined(__linux) || defined(_AIX)
    _nproc = static_cast<int>(sysconf(_SC_NPROCESSORS_ONLN));
#elif defined(__APPLE__)
    static int ncpu[2] = { CTL_HW, HW_NCPU };
    size_t sz = sizeof(_nproc);
    if(sysctl(ncpu, 2, &_nproc, &sz, 0, 0) == -1)
    {
	Ice::SyscallException ex(__FILE__, __LINE__);
	ex.error = getSystemErrno();
	throw ex;
    }
#else
    _nproc = 1;
#endif
    assert(_nproc > 0);
#endif
}

NodeI::~NodeI()
{
#ifdef _WIN32
    PdhCloseQuery(_query);
#endif
}

ServerPrx
NodeI::loadServer(const ServerDescriptorPtr& desc, 
		  StringAdapterPrxDict& adapters, 
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
    proxy->update(desc, init, adapters, activationTimeout, deactivationTimeout);
    
    for(PatchDescriptorSeq::const_iterator p = desc->patchs.begin(); p != desc->patchs.end(); ++p)
    {
	string dest = p->destination.empty() ? _serversDir + "/" + desc->id + "/data" : p->destination;
	PatchDirectory& patch = _directories[dest];
	patch.proxy = _adapter->getCommunicator()->stringToProxy(p->proxy);
	patch.directories.insert(p->sources.begin(), p->sources.end());
	patch.servers.insert(ServerIPtr::dynamicCast(servant));
    }
    
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
	    ServerDescriptorPtr desc = proxy->getDescriptor();
	    proxy->destroy();
	    for(PatchDescriptorSeq::const_iterator p = desc->patchs.begin(); p != desc->patchs.end(); ++p)
	    {
		string dest = p->destination.empty() ? _serversDir + "/" + desc->id + "/data" : p->destination;
		PatchDirectory& patch = _directories[dest];
		for(Ice::StringSeq::const_iterator q = p->sources.begin(); q != p->sources.end(); ++q)
		{
		    patch.directories.erase(*q);
		}
		patch.servers.erase(ServerIPtr::dynamicCast(servant));
		if(patch.servers.empty())
		{
		    _directories.erase(dest);
		}
	    }
	}
	catch(const Ice::LocalException&)
	{
	}
    }
}

void
NodeI::patch(const Ice::StringSeq& directories, const Ice::StringSeq& serverDirs, bool shutdown, const Ice::Current&)
{
    Ice::StringSeq::const_iterator p;
    for(p = directories.begin(); p != directories.end(); ++p)
    {
	patch(ServerIPtr(), *p, shutdown);
    }
    for(p = serverDirs.begin(); p != serverDirs.end(); ++p)
    {
	patch(ServerIPtr(), _serversDir + "/" + *p + "/data", shutdown);
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

void
NodeI::patch(const ServerIPtr& server, const string& directory, bool shutdown) const
{
    Lock sync(*this);

    assert(server || !directory.empty());
    string dest = directory.empty() ? _serversDir + "/" + server->getId() + "/data" : directory;

    map<string, PatchDirectory>::const_iterator p = _directories.find(dest);
    if(p == _directories.end())
    {
	return;
    }
    
    const PatchDirectory& patch = p->second;
    try
    {
	vector<string> running;
	set<ServerIPtr>::const_iterator p;
	for(p = patch.servers.begin(); p != patch.servers.end(); ++p)
	{
	    if(*p != server)
	    {
		if(!(*p)->startUpdating(shutdown))
		{
		    running.push_back((*p)->getId());
		}
	    }
	}
	if(!running.empty())
	{
	    PatchException ex;
	    ex.reason = "patch for `" + dest + "' on node `" + _name + "' failed:\n";
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
	    FileServerPrx icepatch = FileServerPrx::checkedCast(patch.proxy);
	    if(!icepatch)
	    {
		throw "proxy `" + getCommunicator()->proxyToString(patch.proxy) + "' is not a file server.";
	    }
	    
	    const string logdest = dest.find(_serversDir) == 0 ? dest.substr(_serversDir.size() + 1) : dest;
	    PatcherFeedbackPtr feedback = new LogPatcherFeedback(_traceLevels, logdest);
	    PatcherPtr patcher = new Patcher(icepatch, feedback, dest, false, 100, 1);
	    bool aborted = !patcher->prepare();
	    if(!aborted)
	    {
		if(patch.directories.empty())
		{
		    aborted = !patcher->patch("");
		}
		else
		{
		    vector<string> sources;
		    copy(patch.directories.begin(), patch.directories.end(), back_inserter(sources)); 
		    for(vector<string>::const_iterator p = sources.begin(); p != sources.end(); ++p)
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
	catch(const Ice::LocalException& ex)
	{
	    ostringstream os;
	    os << "patch for `" + dest + "' on node `" + _name + "' failed:\n";
	    os << ex;
	    _traceLevels->logger->warning(os.str());
	    throw PatchException(os.str());
	}
	catch(const string& ex)
	{
	    string msg = "patch for `" + dest + "' on node `" + _name + "' failed:\n" + ex;
	    _traceLevels->logger->error(msg);
	    throw PatchException(msg);
	}

	for(p = patch.servers.begin(); p != patch.servers.end(); ++p)
	{
	    if(*p != server)
	    {
		(*p)->finishUpdating();
	    }
	}
    }
    catch(...)
    {
	for(set<ServerIPtr>::const_iterator p = patch.servers.begin(); p != patch.servers.end(); ++p)
	{
	    if(*p != server)
	    {
		(*p)->finishUpdating();
	    }
	}
	throw;
    }
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
	    LoadInfo info;
	    info.load1 = 1.0f;
	    info.load5 = 1.0f;
	    info.load15 = 1.0f;
	    info.nProcessors = 1;

#if defined(_WIN32)
	    //
	    // TODO: Use CPU utilization
	    //
	    int usage = 100;
	    if(PdhCollectQueryData(_query) == ERROR_SUCCESS)
	    {
		DWORD type;
		PDH_FMT_COUNTERVALUE value;
		PdhGetFormattedCounterValue(_counter, PDH_FMT_LONG, &type, &value);
		usage = static_cast<int>(value.longValue);
	    }

	    _last1Total += usage - _usages1.back();
	    _last5Total += usage - _usages5.back();
	    _last15Total += usage - _usages15.back();

	    _usages1.pop_back();
	    _usages5.pop_back();
	    _usages15.pop_back();
	    _usages1.push_front(usage);
	    _usages5.push_front(usage);
	    _usages15.push_front(usage);

	    info.load1 = static_cast<float>(_last1Total) / _usages1.size() / 100.0f;
	    info.load5 = static_cast<float>(_last5Total) / _usages5.size() / 100.0f;
	    info.load15 = static_cast<float>(_last15Total) / _usages15.size() / 100.0f;
	    info.nProcessors = 1;
#elif defined(__sun) || defined(__linux) || defined(__APPLE__)
	    //
	    // We use the load average divided by the number of
	    // processors to figure out if the machine is busy or
	    // not. The result is capped at 1.0f.
	    //
	    double loadAvg[3];
	    if(getloadavg(loadAvg, 3) != -1)
	    {
		info.load1 = static_cast<float>(loadAvg[0]);
		info.load5 = static_cast<float>(loadAvg[1]);
		info.load15 = static_cast<float>(loadAvg[2]);
	    }
	    info.nProcessors =  _nproc;
#elif defined(__hpux)
	    struct pst_dynamic dynInfo;
	    if(pstat_getdynamic(&dynInfo, sizeof(dynInfo), 1, 0) >= 0)
	    {
		info.load1 = dynInfo.psd_avg_1_min;
		info.load5 = dynInfo.psd_avg_5_min;
		info.load15 = dynInfo.psd_avg_15_min;
		info.nProcessors = dynInfo.psd_proc_cnt;
	    }
#elif defined(_AIX)
	    struct nlist nl;
	    nl.n_name = "avenrun";
	    nl.n_value = 0;
	    int kmem;
	    if(knlist(&nl, 1, sizeof(nl)) != 0 && (kmem = open("/dev/kmem", 0, 0)) > 0)
	    {
		long avenrun[3];
		if(pread(kmem, avenrun, sizeof(avenrun), nl.n_value) >= sizeof(avenrun))
		{
		    info.load1 = avenrun[0] / 65535.0f;
		    info.load5 = avenrun[1] / 65535.0f;
		    info.load15 = avenrun[2] / 65535.0f;
		    cerr << info.load1 << endl;
		}
		close(kmem);
	    }
	    info.nProcessors =  _nproc;
#endif
	    session->keepAlive(info);
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
	    Ice::ObjectPrx registry = getCommunicator()->stringToProxy("IceGrid/Registry@IceGrid.Registry.Internal");
	    NodeObserverPrx observer;
	    setSession(RegistryPrx::uncheckedCast(registry)->registerNode(_name, _proxy, observer), observer);
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
	info.servers = serverInfos;
	info.adapters = adapterInfos;
	_observer->nodeUp(info);
    }
    catch(const Ice::LocalException&)
    {
    }
}
