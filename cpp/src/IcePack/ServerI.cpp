// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifdef __sun
#define _POSIX_PTHREAD_SEMANTICS
#endif

#include <IceUtil/UUID.h>
#include <Ice/Ice.h>
#include <IcePack/ServerI.h>
#include <IcePack/ServerFactory.h>
#include <IcePack/TraceLevels.h>
#include <IcePack/Activator.h>

#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32
#   include <direct.h>
#else
#   include <unistd.h>
#   include <dirent.h>
#endif

#include <fstream>

using namespace std;
using namespace IcePack;

ServerI::ServerI(const ServerFactoryPtr& factory, 
		 const TraceLevelsPtr& traceLevels, 
		 const ActivatorPtr& activator,
		 Ice::Int waitTime,
		 const string& serversDir) :
    _factory(factory),
    _traceLevels(traceLevels),
    _activator(activator),
    _waitTime(waitTime),
    _serversDir(serversDir),
    _state(Inactive)
{
    assert(_activator);
}

ServerI::~ServerI()
{
}

bool
ServerI::start(ServerActivation act, const Ice::Current& current)
{
    string exe;
    string wd;
    Ice::StringSeq opts;
    Ice::StringSeq evs;

    while(true)
    {
	Lock sync(*this);
	switch(_state)
	{
	case Inactive:
	{
	    if(act < activation)
	    {
	        return false;
	    }

	    _state = Activating;

	    //
	    // Prevent eviction of the server object once it's not anymore in the inactive state.
	    //
	    _factory->getServerEvictor()->keep(current.id);

	    break;
	}
	case Activating:
	case Deactivating:
	{
	    wait(); // TODO: Timeout?
	    continue;
	}
 	case Active:
	{
	    return true; // Raise an exception instead?
	}
	case Destroying:
	case Destroyed:
	{
	    Ice::ObjectNotExistException ex(__FILE__,__LINE__);
	    ex.id = current.id;
	    throw ex;
	}
	}

	if(_traceLevels->server > 2)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->serverCat);
	    out << "changed server `" << name << "' state to `Activating'";
	}
	assert(_state == Activating);

	exe = exePath;
	wd = pwd;
	opts = options;
	evs = envs;
	break;
    }

    try
    {
	ServerPrx self = ServerPrx::uncheckedCast(current.adapter->createProxy(current.id));
	bool active  = _activator->activate(name, exe, wd, opts, evs, self);
	setState(active ? Active : Inactive, current);
	return active;
    }
    catch(const Ice::SyscallException& ex)
    {
	Ice::Warning out(_traceLevels->logger);
	out << "activation failed for server `" << name << "':\n";
	out << ex;

	setState(Inactive, current);
	return false;
    }
}

void
ServerI::stop(const Ice::Current& current)
{
    while(true)
    {
	Lock sync(*this);
	switch(_state)
	{
	case Inactive:
	{
	    return;
	}
	case Activating:
	case Deactivating:
	{
	    wait(); // TODO: Timeout?
	    continue;
	}
 	case Active:
	{	    
	    _state = Deactivating;
	    break;
	}
	case Destroying:
	case Destroyed:
	{
	    Ice::ObjectNotExistException ex(__FILE__,__LINE__);
	    ex.id = current.id;
	    throw ex;
	}
	}

	if(_traceLevels->server > 2)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->serverCat);
	    out << "changed server `" << name << "' state to `Deactivating'";
	}
	assert(_state == Deactivating);
	break;
    }

    stopInternal(current);
}

void
ServerI::sendSignal(const string& signal, const Ice::Current& current)
{
    _activator->sendSignal(name, signal);
}

void
ServerI::writeMessage(const string& message, Ice::Int fd, const Ice::Current& current)
{
    IceUtil::Monitor< ::IceUtil::Mutex>::Lock sync(*this);
    if(_process != 0)
    {
	try
	{
	    _process->writeMessage(message, fd);
	}
	catch(const Ice::LocalException&)
	{
	}
    }
}

void
ServerI::destroy(const Ice::Current& current)
{
    bool stop = false;

    while(true)
    {
	Lock sync(*this);
	switch(_state)
	{
	case Inactive:
	{
	    _state = Destroyed;
	    break;
	}
 	case Active:
	{
	    stop = true;
	    _state = Destroying;
	    break;
	}
	case Activating:
	case Deactivating:
	{
	    wait(); // TODO: Timeout?
	    continue;
	}
	case Destroying:
	case Destroyed:
	{
	    Ice::ObjectNotExistException ex(__FILE__,__LINE__);
	    ex.id = current.id;
	    throw ex;
	}
	}

	assert(_state == Destroyed || _state == Destroying);

	if(_traceLevels->server > 2)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->serverCat);
	    out << "changed server `" << name << "' state to `";
	    out << (_state == Destroyed ? "Destroyed" : "Destroying") << "'";
	}
	break;
    }

    if(stop)
    {
	stopInternal(current);
    }

    
    _factory->destroy(this, current.id);
}

void
ServerI::terminated(const Ice::Current& current)
{
    ServerState newState = Inactive; // Initialize to keep the compiler happy.
    ServerAdapterPrxDict adpts;
    while(true)
    {
	Lock sync(*this);
	switch(_state)
	{
	case Inactive:
	{
	    assert(false);
	}
	case Activating:
	{
	    wait(); // TODO: Timeout?
	    continue;
	}
	case Active:
	{
	    _state = Deactivating;
	    if(_traceLevels->server > 2)
	    {
		Ice::Trace out(_traceLevels->logger, _traceLevels->serverCat);
		out << "changed server `" << name << "' state to `Deactivating'";
	    }
	    newState = Inactive;
	    break;
	}
	case Deactivating:
	{
	    //
	    // Deactivation was initiated by the stop method.
	    //
	    newState = Inactive;
	    break;
	}
	case Destroying:
	{
	    //
	    // Deactivation was initiated by the destroy method.
	    //
	    newState = Destroyed;
	    break;
	}
	case Destroyed:
	{
	    assert(false);
	}
	}

	assert(_state == Deactivating || _state == Destroying);
	adpts = adapters;

	//
	// Clear the process proxy.
	//
	_process = 0;
	break;
    }

    if(newState != Destroyed)
    {
	//
	// The server has terminated, set its adapter direct proxies to
	// null to cause the server re-activation if one of its adapter
	// direct proxy is requested.
	//
	for(ServerAdapterPrxDict::iterator p = adpts.begin(); p != adpts.end(); ++p)
	{
	    try
	    {
		p->second->setDirectProxy(0);
	    }
	    catch(const Ice::ObjectNotExistException&)
	    {
	    }
	}
    }

    setState(newState, current);
}

ServerState
ServerI::getState(const Ice::Current&)
{
    Lock sync(*this);
    return _state;
}

Ice::Int
ServerI::getPid(const Ice::Current& current)
{
    return _activator->getServerPid(name);
}

void 
ServerI::setActivationMode(ServerActivation mode, const ::Ice::Current&)
{
    Lock sync(*this);
    activation = mode;
}

ServerActivation 
ServerI::getActivationMode(const ::Ice::Current&)
{
    Lock sync(*this);
    return activation;
}

ServerDescriptorPtr
ServerI::getDescriptor(const Ice::Current&)
{
    Lock sync(*this);
    return descriptor;
}

void 
ServerI::setExePath(const string& path, const ::Ice::Current&)
{
    Lock sync(*this);
    exePath = path;
}

void 
ServerI::setPwd(const string& path,const ::Ice::Current&)
{
    Lock sync(*this);
    pwd = path;
}

void 
ServerI::setEnvs(const Ice::StringSeq& s, const ::Ice::Current&)
{
    Lock sync(*this);
    envs = s;
}

void 
ServerI::setOptions(const Ice::StringSeq& opts, const ::Ice::Current&)
{
    Lock sync(*this);
    options = opts;
}

void 
ServerI::addAdapter(const ServerAdapterPrx& adapter, bool registerProcess, const ::Ice::Current&)
{
    Lock sync(*this);
    ServerAdapterPrxDict::const_iterator p = adapters.find(adapter->ice_getIdentity());
    if(p != adapters.end())
    {
	DeploymentException ex;
	ex.reason = "failed to add adapter because it already exists";
	throw ex;
    }
    adapters[adapter->ice_getIdentity()] = adapter;
    processRegistered |= registerProcess;
}

void
ServerI::removeAdapter(const ServerAdapterPrx& adapter, const ::Ice::Current&)
{
    Lock sync(*this);
    adapters.erase(adapter->ice_getIdentity());
}

string
ServerI::addConfigFile(const string& n, const PropertyDescriptorSeq& properties, const ::Ice::Current&)
{
    string file = _serversDir + name + "/config/" + n;

    ofstream configfile;
    configfile.open(file.c_str(), ios::out);
    if(!configfile)
    {
	DeploymentException ex;
	ex.reason = "couldn't create configuration file: " + file;
	throw ex;
    }

    for(PropertyDescriptorSeq::const_iterator p = properties.begin(); p != properties.end(); ++p)
    {
	configfile << p->name;
	if(!p->value.empty())
	{
	    configfile << "=" << p->value;
	}
	configfile << endl;
    }
    configfile.close();
    
    return file;
}

void 
ServerI::removeConfigFile(const string& n, const ::Ice::Current&)
{
    string file = _serversDir + name + "/config/" + n;
    if(unlink(file.c_str()) != 0)
    {
	Ice::Warning out(_traceLevels->logger);
	out << "couldn't remove configuration file: " + file + ": " + strerror(getSystemErrno());
    }
}

string
ServerI::addDbEnv(const DbEnvDescriptor& dbEnv, const string& path, const ::Ice::Current&)
{
    string dir;
    if(dbEnv.dbHome.empty())
    {
	dir = _serversDir + name + "/dbs/" + dbEnv.name;
    }
    else
    {
	dir = dbEnv.dbHome;
    }

    //
    // If no db home directory is specified for this db env, we provide one.
    //
    if(dbEnv.dbHome.empty())
    {
	//
	// First, we try to move the given backup if specified, if not successful, we just
	// create the database environment directory.
	//
	if(path.empty() || rename((path + "/" + dbEnv.name).c_str(), dir.c_str()) != 0)
	{
	    //
	    // Create the database environment directory.
	    //
#ifdef _WIN32
	    if(_mkdir(dir.c_str()) != 0)
#else
	    if(mkdir(dir.c_str(), 0755) != 0)
#endif
	    {
		DeploymentException ex;
		ex.reason = "couldn't create directory " + dir + ": " + strerror(getSystemErrno());
		throw ex;
	    }
	}
    }

    string file = dir + "/DB_CONFIG";
    ofstream configfile;
    configfile.open(file.c_str(), ios::out);
    if(!configfile)
    {
	rmdir(dir.c_str());

	DeploymentException ex;
	ex.reason = "couldn't create configuration file: " + file;
	throw ex;
    }

    for(PropertyDescriptorSeq::const_iterator p = dbEnv.properties.begin(); p != dbEnv.properties.end(); ++p)
    {
	if(!p->name.empty())
	{
	    configfile << p->name;
	    if(!p->value.empty())
	    {
		configfile << " " << p->value;
	    }
	    configfile << endl;
	}
    }
    configfile.close();

    return dir;
}

void 
ServerI::removeDbEnv(const DbEnvDescriptor& dbEnv, const string& moveTo, const ::Ice::Current&)
{
    string path;
    if(dbEnv.dbHome.empty())
    {
	path = _serversDir + name + "/dbs/" + dbEnv.name;
    }
    else
    {
	path = dbEnv.dbHome;
    }

    if(unlink((path + "/DB_CONFIG").c_str()) != 0)
    {
	Ice::Warning out(_traceLevels->logger);
	out << "couldn't remove file: " + path + "/DB_CONFIG: " + strerror(getSystemErrno());
    }

    //
    // If no db home directory was specified for this db env, we provided one. We need to cleanup
    // this directory now.
    //
    if(dbEnv.dbHome.empty())
    {
	if(!moveTo.empty())
	{
	    //
	    // Move the database environment directory to the given directory.
	    //
	    if(rename(path.c_str(), (moveTo + "/" + dbEnv.name).c_str()) != 0)
	    {
		Ice::Warning out(_traceLevels->logger);
		out << "couldn't rename directory " + path + " to " + moveTo + "/" + dbEnv.name + ": " + 
		    strerror(getSystemErrno());
	    }
	}
	else
	{
	    //
	    // Delete the database environment directory.
	    //
	    Ice::StringSeq files;
	    
#ifdef _WIN32
	    string pattern = path + "/*";
	    WIN32_FIND_DATA data;
	    HANDLE hnd = FindFirstFile(pattern.c_str(), &data);
	    if(hnd == INVALID_HANDLE_VALUE)
	    {
		// TODO: log a warning, throw an exception?
		return;
	    }
	    
	    do
	    {
		if((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		{
		    files.push_back(path + "/" + data.cFileName);
		}
	    } 
	    while(FindNextFile(hnd, &data));
	    
	    FindClose(hnd);
#else
	    
	    DIR* dir = opendir(path.c_str());
	    if(dir == 0)
	    {
		// TODO: log a warning, throw an exception?
		return;
	    }
	    
	    // TODO: make the allocation/deallocation exception-safe
	    struct dirent* entry = static_cast<struct dirent*>(malloc(pathconf(path.c_str(), _PC_NAME_MAX) + 1));
	    
	    while(readdir_r(dir, entry, &entry) == 0 && entry != 0)
	    {
		string name = path + "/" + entry->d_name;
		struct stat buf;
		
		if(::stat(name.c_str(), &buf) != 0)
		{
		    if(errno != ENOENT)
		    {
			//
			// TODO: log error
			//
		    }
		}
		else if(S_ISREG(buf.st_mode))
		{
		    files.push_back(name);
		}
	    }
	    
	    free(entry);
	    closedir(dir);
#endif
	    
	    for(Ice::StringSeq::iterator p = files.begin(); p != files.end(); ++p)
	    {
		if(unlink(p->c_str()) != 0)
		{
		    //
		    // TODO: log error
		    //
		}
	    }
	    
	    if(rmdir(path.c_str()) != 0)
	    {
		Ice::Warning out(_traceLevels->logger);
		out << "couldn't remove directory: " + path + ": " + strerror(getSystemErrno());
	    }
	}
    }
}

void
ServerI::setProcess(const ::Ice::ProcessPrx& proc, const ::Ice::Current&)
{
    IceUtil::Monitor< ::IceUtil::Mutex>::Lock sync(*this);
    _process = proc;
    notifyAll();
}

void
ServerI::stopInternal(const Ice::Current& current)
{
    Ice::ProcessPrx process;
    {
	IceUtil::Monitor< ::IceUtil::Mutex>::Lock sync(*this);
	if(!_process && processRegistered)
	{
	    while(!_process)
	    {
		if(_state == Inactive || _state == Destroyed)
		{
		    //
		    // State changed to inactive or destroyed, the server
		    // has been correctly deactivated, we can return.
		    //
		    return;
		}

		//
		// Wait for the process to be set.
		//
		wait(); // TODO: timeout?
	    }
	}
	process = _process;
    }

    try
    {
	//
	// Deactivate the server.
	//
	_activator->deactivate(name, process);

	//
	// Wait for the server to be inactive (the activator monitors
	// the process and should notify us when it detects the
	// process termination by calling the terminated() method).
	//
	Lock sync(*this);

#ifndef NDEBUG
	ServerState oldState = _state;
#endif

	while(true)
	{
	    if(_state == Inactive || _state == Destroyed)
	    {
		//
		// State changed to inactive or destroyed, the server
		// has been correctly deactivated, we can return.
		//
		return;
	    }
	    
	    //
	    // Wait for a notification.
	    //
	    bool notify = timedWait(IceUtil::Time::seconds(_waitTime));
	    if(!notify)
	    {
		assert(oldState == _state);
		break;
	    }
	}

	if(_traceLevels->server > 1)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->serverCat);
	    out << "graceful server shutdown timed out, killing server `" << name << "'";
	}
    }
    catch(const Ice::Exception& ex)
    {
	Ice::Warning out(_traceLevels->logger);
	out << "graceful server shutdown failed, killing server `" << name << "':\n";
	out << ex;
    }

    
    //
    // The server is still not inactive, kill it.
    //
    try
    {
	_activator->kill(name);
    }
    catch(const Ice::SyscallException& ex)
    {
	Ice::Warning out(_traceLevels->logger);
	out << "deactivation failed for server `" << name << "':\n";
	out << ex;
	
	setState(Active, current);
    }
}

void
ServerI::setState(ServerState st, const Ice::Current& current)
{
    Lock sync(*this);

    //
    // Allow eviction of an inactive server object.
    //
    if(_state != Inactive && st == Inactive)
    {
	_factory->getServerEvictor()->release(current.id);
    }

    _state = st;

    if(_traceLevels->server > 1)
    {
	if(_state == Active)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->serverCat);
	    out << "changed server `" << name << "' state to `Active'";
	}
	else if(_state == Inactive)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->serverCat);
	    out << "changed server `" << name << "' state to `Inactive'";
	}
	else if(_state == Destroyed)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->serverCat);
	    out << "changed server `" << name << "' state to `Destroyed'";
	}
	else if(_traceLevels->server > 2)
	{
	    if(_state == Activating)
	    {
		Ice::Trace out(_traceLevels->logger, _traceLevels->serverCat);
		out << "changed server `" << name << "' state to `Activating'";
	    }
	    else if(_state == Deactivating)
	    {
		Ice::Trace out(_traceLevels->logger, _traceLevels->serverCat);
		out << "changed server `" << name << "' state to `Deactivating'";
	    }
	}
    }

    notifyAll();
}
