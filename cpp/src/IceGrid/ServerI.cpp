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

#include <Ice/Ice.h>
#include <IceGrid/ServerI.h>
#include <IceGrid/TraceLevels.h>
#include <IceGrid/Activator.h>
#include <IceGrid/NodeI.h>
#include <IceGrid/Util.h>
#include <IceGrid/ServerAdapterI.h>

#include <IcePatch2/Util.h>

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
using namespace IceGrid;
using namespace IcePatch2;

ServerI::ServerI(const NodeIPtr& node, const string& serversDir, const string& name) :
    _node(node),
    _name(name),
    _waitTime(_node->getCommunicator()->getProperties()->getPropertyAsIntWithDefault("IceGrid.Node.WaitTime", 60)),
    _serversDir(serversDir),
    _state(Inactive)
{
    assert(_node->getActivator());
}

ServerI::~ServerI()
{
}

void
ServerI::load(const ServerDescriptorPtr& descriptor, StringAdapterPrxDict& adapters, const Ice::Current& current)
{
    while(true)
    {
	{
	    Lock sync(*this);
	    if(_state == Destroying || _state == Destroyed)
	    {
		Ice::ObjectNotExistException ex(__FILE__,__LINE__);
		ex.id = current.id;
		throw ex;
	    }
	    else if(_state == Inactive)
	    {
		//
		// If the server is inactive we can update its descriptor and its directory.
		//
		try
		{
		    update(descriptor, adapters, current);
		}
		catch(const string& msg)
		{
		    DeploymentException ex;
		    ex.reason = msg;
		    throw ex;
		}
		return;
	    }
	}

	//
	// If the server is not inactive we stop it and try again to update it.
	//
	stop(current);
    }
}

bool
ServerI::start(ServerActivation act, const Ice::Current& current)
{
    ServerDescriptorPtr desc;
    while(true)
    {
	Lock sync(*this);
	switch(_state)
	{
	case Inactive:
	{
	    if(act < _activation)
	    {
	        return false;
	    }

	    setStateNoSync(Activating, current);
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

	assert(_state == Activating);

	desc = _desc;
	break;
    }

    //
    // Compute the server command line options.
    //
    Ice::StringSeq options;
    string exe;
    if(!desc->interpreter.empty())
    {
	if(desc->interpreter == "icebox")
	{
	    exe = desc->exe.empty() ? "icebox" : desc->exe;
	    copy(desc->interpreterOptions.begin(), desc->interpreterOptions.end(), back_inserter(options));	    
	}
	else if(desc->interpreter == "java-icebox")
	{
	    exe = desc->exe.empty() ? "java" : desc->exe;
	    copy(desc->interpreterOptions.begin(), desc->interpreterOptions.end(), back_inserter(options));
	    options.push_back("IceBox.Server");
	}
	else
	{
	    exe = desc->interpreter;
	    copy(desc->interpreterOptions.begin(), desc->interpreterOptions.end(), back_inserter(options));
	    options.push_back(desc->exe);
	}
    }
    else
    {
	exe = desc->exe;
    }
    copy(desc->options.begin(), desc->options.end(), back_inserter(options));
    options.push_back("--Ice.Config=" + _serverDir + "/config/config");

    Ice::StringSeq envs;
    copy(desc->envs.begin(), desc->envs.end(), back_inserter(envs));

    try
    {
	ServerPrx self = ServerPrx::uncheckedCast(current.adapter->createProxy(current.id));
	bool active  = _node->getActivator()->activate(desc->name, exe, desc->pwd, options, envs, self);
	setState(active ? Active : Inactive, current);
	return active;
    }
    catch(const Ice::SyscallException& ex)
    {
	Ice::Warning out(_node->getTraceLevels()->logger);
	out << "activation failed for server `" << _name << "':\n";
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
	    setStateNoSync(Deactivating, current);
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

	assert(_state == Deactivating);
	break;
    }

    stopInternal(false, current);
}

void
ServerI::sendSignal(const string& signal, const Ice::Current& current)
{
    _node->getActivator()->sendSignal(_name, signal);
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
	    setStateNoSync(Destroyed, current);
	    break;
	}
 	case Active:
	{
	    stop = true;
	    setStateNoSync(Destroying, current);
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
	break;
    }

    if(stop)
    {
	stopInternal(true, current);
    }

    //
    // Destroy the object adapters.
    //
    for(StringAdapterPrxDict::const_iterator p = _adapters.begin(); p != _adapters.end(); ++p)
    {
	try
	{
	    p->second->destroy();
	}
	catch(const Ice::LocalException&)
	{
	}
    }

    //
    // Delete the server directory from the disk.
    //
    try
    {
	removeRecursive(_serverDir);
    }
    catch(const string&)
    {
	// TODO: warning?
    }
    
    //
    // Unregister from the object adapter.
    //
    current.adapter->remove(current.id);
}

void
ServerI::terminated(const Ice::Current& current)
{
    ServerState newState = Inactive; // Initialize to keep the compiler happy.
    StringAdapterPrxDict adpts;
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
	    setStateNoSync(Deactivating, current);
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
	adpts = _adapters;

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
	for(StringAdapterPrxDict::iterator p = adpts.begin(); p != adpts.end(); ++p)
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
    return _node->getActivator()->getServerPid(_name);
}

void 
ServerI::setActivationMode(ServerActivation mode, const ::Ice::Current&)
{
    Lock sync(*this);
    _activation = mode;
}

ServerActivation 
ServerI::getActivationMode(const ::Ice::Current&)
{
    Lock sync(*this);
    return _activation;
}

ServerDescriptorPtr
ServerI::getDescriptor(const Ice::Current&)
{
    Lock sync(*this);
    return _desc;
}

void
ServerI::setProcess(const ::Ice::ProcessPrx& proc, const ::Ice::Current&)
{
    IceUtil::Monitor< ::IceUtil::Mutex>::Lock sync(*this);
    _process = proc;
    notifyAll();
}

StringAdapterPrxDict
ServerI::getAdapters(const Ice::Current&)
{
    Lock sync(*this);
    return _adapters;
}

void
ServerI::stopInternal(bool kill, const Ice::Current& current)
{
    Ice::ProcessPrx process;
    if(!kill)
    {
	IceUtil::Monitor< ::IceUtil::Mutex>::Lock sync(*this);
	if(!_process && _processRegistered)
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
	if(kill)
	{
	    _node->getActivator()->kill(_name);
	}
	else
	{
	    _node->getActivator()->deactivate(_name, process);
	}

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

	if(_node->getTraceLevels()->server > 1)
	{
	    Ice::Trace out(_node->getTraceLevels()->logger, _node->getTraceLevels()->serverCat);
	    out << "graceful server shutdown timed out, killing server `" << _name << "'";
	}
    }
    catch(const Ice::Exception& ex)
    {
	Ice::Warning out(_node->getTraceLevels()->logger);
	out << "graceful server shutdown failed, killing server `" << _name << "':\n";
	out << ex;
    }

    
    //
    // The server is still not inactive, kill it.
    //
    try
    {
	_node->getActivator()->kill(_name);
    }
    catch(const Ice::SyscallException& ex)
    {
	Ice::Warning out(_node->getTraceLevels()->logger);
	out << "deactivation failed for server `" << _name << "':\n";
	out << ex;
	
	setState(Active, current);
    }
}

void
ServerI::setState(ServerState st, const Ice::Current& current)
{
    Lock sync(*this);
    setStateNoSync(st, current);
    notifyAll();
}

void
ServerI::setStateNoSync(ServerState st, const Ice::Current& current)
{
    _state = st;

    NodeObserverPrx observer = _node->getObserver();
    if(observer)
    {
	ServerDynamicInfo info;
	info.name = _name;
	info.state = st;
	//
	// NOTE: this must be done only for the active state. 
	// Otherwise, we could get a deadlock since getPid()
	// will lock the activator and since this method might 
	// be called from the activator locked.
	//
	info.pid = st == Active ? getPid(current) : 0;

	try
	{
	    observer->updateServer(_node->getName(current), info);
	}
	catch(const Ice::LocalException&)
	{
	}
    }

    if(_node->getTraceLevels()->server > 1)
    {
	if(_state == Active)
	{
	    Ice::Trace out(_node->getTraceLevels()->logger, _node->getTraceLevels()->serverCat);
	    out << "changed server `" << _name << "' state to `Active'";
	}
	else if(_state == Inactive)
	{
	    Ice::Trace out(_node->getTraceLevels()->logger, _node->getTraceLevels()->serverCat);
	    out << "changed server `" << _name << "' state to `Inactive'";
	}
	else if(_state == Destroyed)
	{
	    Ice::Trace out(_node->getTraceLevels()->logger, _node->getTraceLevels()->serverCat);
	    out << "changed server `" << _name << "' state to `Destroyed'";
	}
	else if(_node->getTraceLevels()->server > 2)
	{
	    if(_state == Activating)
	    {
		Ice::Trace out(_node->getTraceLevels()->logger, _node->getTraceLevels()->serverCat);
		out << "changed server `" << _name << "' state to `Activating'";
	    }
	    else if(_state == Deactivating)
	    {
		Ice::Trace out(_node->getTraceLevels()->logger, _node->getTraceLevels()->serverCat);
		out << "changed server `" << _name << "' state to `Deactivating'";
	    }
	    else if(_state == Destroying)
	    {
		Ice::Trace out(_node->getTraceLevels()->logger, _node->getTraceLevels()->serverCat);
		out << "changed server `" << _name << "' state to `Destroying'";
	    }
	}
    }
}

void
ServerI::update(const ServerDescriptorPtr& descriptor, StringAdapterPrxDict& adapters, const Ice::Current& current)
{
    ServerPrx self = ServerPrx::uncheckedCast(current.adapter->createProxy(current.id));

    _desc = descriptor;
    _serverDir = _serversDir + "/" + descriptor->name;
    _activation = descriptor->activation  == "on-demand" ? OnDemand : Manual;
    
    //
    // Make sure the server directories exists.
    //
    try
    {
	Ice::StringSeq contents = readDirectory(_serverDir);
	if(find(contents.begin(), contents.end(), "config") == contents.end())
	{
	    throw "can't find `config' directory in `" + _serverDir + "'";
	}
	if(find(contents.begin(), contents.end(), "dbs") == contents.end())
	{
	    throw "can't find `dbs' directory in `" + _serverDir + "'";
	}
    }
    catch(const string& message)
    {
	//
	// TODO: log message?
	//

	createDirectory(_serverDir);
	createDirectory(_serverDir + "/config");
	createDirectory(_serverDir + "/dbs");
    }

    //
    // Update the configuration file(s) of the server if necessary.
    //
    Ice::StringSeq knownFiles;
    updateConfigFile(_serverDir, descriptor);
    knownFiles.push_back("config");
    IceBoxDescriptorPtr iceBox = IceBoxDescriptorPtr::dynamicCast(descriptor);
    if(iceBox)
    {
	for(InstanceDescriptorSeq::const_iterator p = iceBox->services.begin(); p != iceBox->services.end(); ++p)
	{
	    updateConfigFile(_serverDir, ServiceDescriptorPtr::dynamicCast(p->descriptor));
	    knownFiles.push_back("config_" + p->descriptor->name);
	}
    }

    //
    // Remove old configuration files.
    //
    Ice::StringSeq configFiles = readDirectory(_serverDir + "/config");
    Ice::StringSeq toDel;
    set_difference(configFiles.begin(), configFiles.end(), knownFiles.begin(), knownFiles.end(), back_inserter(toDel));
    for(Ice::StringSeq::const_iterator p = toDel.begin(); p != toDel.end(); ++p)
    {
	if(p->find("config_") == 0)
	{
	    try
	    {
		remove(_serverDir + "/config/" + *p);
	    }
	    catch(const string& message)
	    {
		//
		// TODO: warning
		//
	    }
	}
    }

    //
    // Update the database environments if necessary.
    //
    Ice::StringSeq knownDbEnvs;
    for(DbEnvDescriptorSeq::const_iterator p = descriptor->dbEnvs.begin(); p != descriptor->dbEnvs.end(); ++p)
    {
	updateDbEnv(_serverDir, *p);
	knownDbEnvs.push_back(p->name);
    }

    //
    // Remove old database environments.
    //
    Ice::StringSeq dbEnvs = readDirectory(_serverDir + "/dbs");
    toDel.clear();
    set_difference(dbEnvs.begin(), dbEnvs.end(), knownDbEnvs.begin(), knownDbEnvs.end(), back_inserter(toDel));
    for(Ice::StringSeq::const_iterator p = toDel.begin(); p != toDel.end(); ++p)
    {
	try
	{
	    removeRecursive(_serverDir + "/dbs/" + *p);
	}
	catch(const string&)
	{
	    //
	    // TODO: warning
	    //
	}
    }

    //
    // Create the object adapter objects if necessary.
    //
    _processRegistered = false;
    StringAdapterPrxDict oldAdapters;
    oldAdapters.swap(_adapters);
    for(AdapterDescriptorSeq::const_iterator p = descriptor->adapters.begin(); p != descriptor->adapters.end(); ++p)
    {
	addAdapter(*p, self, current);
	oldAdapters.erase(p->id);
    }
    if(iceBox)
    {
	for(InstanceDescriptorSeq::const_iterator p = iceBox->services.begin(); p != iceBox->services.end(); ++p)
	{
	    ServiceDescriptorPtr s = ServiceDescriptorPtr::dynamicCast(p->descriptor);
	    for(AdapterDescriptorSeq::const_iterator q = s->adapters.begin(); q != s->adapters.end(); ++q)
	    {
		addAdapter(*q, self, current);
		oldAdapters.erase(q->id);
	    }
	}
    }
    for(StringAdapterPrxDict::const_iterator p = oldAdapters.begin(); p != oldAdapters.end(); ++p)
    {
	try
	{
	    p->second->destroy();
	}
	catch(const Ice::LocalException&)
	{
	}
    }
    adapters = _adapters;
}

void
ServerI::addAdapter(const AdapterDescriptor& descriptor, const ServerPrx& self, const Ice::Current& current)
{
    Ice::Identity id;
    id.category = "IceGridServerAdapter";
    id.name = _desc->name + "-" + descriptor.id;
    if(!current.adapter->find(id))
    {
	current.adapter->add(new ServerAdapterI(_node, self, descriptor.id, _waitTime), id);
    }
    _adapters[descriptor.id] = AdapterPrx::uncheckedCast(current.adapter->createProxy(id));
}

void
ServerI::updateConfigFile(const string& serverDir, const ComponentDescriptorPtr& descriptor)
{
    string configFilePath;

    PropertyDescriptorSeq props;
    if(ServerDescriptorPtr::dynamicCast(descriptor))
    {
	configFilePath = serverDir + "/config/config";

	//
	// Add server properties.
	//
	props.push_back(createProperty("# Server configuration"));
	props.push_back(createProperty("Ice.ProgramName", descriptor->name));
	copy(descriptor->properties.begin(), descriptor->properties.end(), back_inserter(props));

	//
	// Add service properties.
	//
	string servicesStr;
	IceBoxDescriptorPtr iceBox = IceBoxDescriptorPtr::dynamicCast(descriptor);
	if(iceBox)
	{
	    for(InstanceDescriptorSeq::const_iterator p = iceBox->services.begin(); p != iceBox->services.end(); ++p)
	    {
		ServiceDescriptorPtr s = ServiceDescriptorPtr::dynamicCast(p->descriptor);
		const string path = serverDir + "/config/config_" + s->name;
		props.push_back(createProperty("IceBox.Service." + s->name, s->entry + " --Ice.Config=" + path));
		servicesStr += s->name + " ";
	    }
	    props.push_back(createProperty("IceBox.LoadOrder", servicesStr));
	}
    }
    else
    {
	assert(ServiceDescriptorPtr::dynamicCast(descriptor));
	configFilePath = serverDir + "/config/config_" + descriptor->name;
	props.push_back(createProperty("# Service configuration"));
	copy(descriptor->properties.begin(), descriptor->properties.end(), back_inserter(props));
    }

    //
    // Add database environment properties.
    //
    for(DbEnvDescriptorSeq::const_iterator p = descriptor->dbEnvs.begin(); p != descriptor->dbEnvs.end(); ++p)
    {
	const string path = serverDir + "/dbs/" + p->name;
	props.push_back(createProperty("# Database environment " + p->name));
	props.push_back(createProperty("Freeze.DbEnv." + p->name + ".DbHome", path));
    }

    //
    // Add object adapter properties.
    //
    for(AdapterDescriptorSeq::const_iterator p = descriptor->adapters.begin(); p != descriptor->adapters.end(); ++p)
    {
	props.push_back(createProperty("# Object adapter " + p->name));
	props.push_back(createProperty(p->name + ".Endpoints", p->endpoints));
	props.push_back(createProperty(p->name + ".AdapterId", p->id));
	if(p->registerProcess)
	{
	    props.push_back(createProperty(p->name + ".RegisterProcess", "1"));
	}
    }

    //
    // Load the previous configuration properties.
    //
    PropertyDescriptorSeq orig;
    try
    {
	Ice::PropertiesPtr origProps = Ice::createProperties();
	origProps->load(configFilePath);
	Ice::PropertyDict all = origProps->getPropertiesForPrefix("");
	for(Ice::PropertyDict::const_iterator p = all.begin(); p != all.end(); ++p)
	{
	    orig.push_back(createProperty(p->first, p->second));
	}
    }
    catch(const Ice::LocalException&)
    {
    }

    // 
    // Only update the properties on the disk if they are different.
    //
    if(set<PropertyDescriptor>(orig.begin(), orig.end()) != set<PropertyDescriptor>(props.begin(), props.end()))
    {
	ofstream configfile;
	configfile.open(configFilePath.c_str(), ios::out);
	if(!configfile)
	{
	    DeploymentException ex;
	    ex.reason = "couldn't create configuration file: " + configFilePath;
	    throw ex;
	}
	
	for(PropertyDescriptorSeq::const_iterator p = props.begin(); p != props.end(); ++p)
	{
	    configfile << p->name;
	    if(!p->value.empty())
	    {
		configfile << "=" << p->value;
	    }
	    configfile << endl;
	}
	configfile.close();
    }
}

void
ServerI::updateDbEnv(const string& serverDir, const DbEnvDescriptor& dbEnv)
{
    string dbEnvHome = dbEnv.dbHome;
    if(dbEnvHome.empty())
    {
	dbEnvHome = serverDir + "/dbs/" + dbEnv.name;
	try
	{
	    IcePatch2::createDirectory(dbEnvHome);
	}
	catch(const string& message)
	{
	}
    }

    //
    // TODO: only write the configuration file if necessary.
    //

    string file = dbEnvHome + "/DB_CONFIG";
    ofstream configfile;
    configfile.open(file.c_str(), ios::out);
    if(!configfile)
    {
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
}

PropertyDescriptor
ServerI::createProperty(const string& name, const string& value)
{
    PropertyDescriptor prop;
    prop.name = name;
    prop.value = value;
    return prop;
}

