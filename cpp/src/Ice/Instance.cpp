// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <Ice/Instance.h>
#include <Ice/TraceLevels.h>
#include <Ice/DefaultsAndOverrides.h>
#include <Ice/RouterInfo.h>
#include <Ice/LocatorInfo.h>
#include <Ice/ReferenceFactory.h>
#include <Ice/ProxyFactory.h>
#include <Ice/ThreadPool.h>
#include <Ice/ConnectionFactory.h>
#include <Ice/ConnectionMonitor.h>
#include <Ice/ObjectFactoryManager.h>
#include <Ice/UserExceptionFactoryManager.h>
#include <Ice/LocalException.h>
#include <Ice/ObjectAdapterFactory.h>
#include <Ice/Exception.h>
#include <Ice/Properties.h>
#include <Ice/LoggerI.h>
#include <Ice/Network.h>
#include <Ice/EndpointFactoryManager.h>
#include <Ice/TcpEndpoint.h>
#include <Ice/UdpEndpoint.h>
#include <Ice/DynamicLibrary.h>
#include <Ice/PluginManagerI.h>
#include <Ice/Initialize.h>

#ifndef _WIN32
#   include <Ice/SysLoggerI.h>
#endif

#ifndef _WIN32
#   include <csignal>
#   include <syslog.h>
#   include <pwd.h>
#   include <sys/types.h>
#endif

using namespace std;
using namespace Ice;
using namespace IceInternal;

int Instance::_globalStateCounter = 0;
IceUtil::Mutex* Instance::_globalStateMutex = new IceUtil::Mutex;
#ifndef _WIN32
string Instance::_identForOpenlog;
#endif

namespace IceUtil
{

extern bool ICE_UTIL_API nullHandleAbort;

};

namespace IceInternal
{

class GlobalStateMutexDestroyer
{
public:
    
    ~GlobalStateMutexDestroyer()
    {
	delete Instance::_globalStateMutex;
	Instance::_globalStateMutex = 0;
    }
};

static GlobalStateMutexDestroyer destroyer;

}

void IceInternal::incRef(Instance* p) { p->__incRef(); }
void IceInternal::decRef(Instance* p) { p->__decRef(); }

PropertiesPtr
IceInternal::Instance::properties()
{
    // No mutex lock, immutable.
    return _properties;
}

LoggerPtr
IceInternal::Instance::logger()
{
    IceUtil::RecMutex::Lock sync(*this);
    return _logger;
}

void
IceInternal::Instance::logger(const LoggerPtr& logger)
{
    IceUtil::RecMutex::Lock sync(*this);
    _logger = logger;
}

StatsPtr
IceInternal::Instance::stats()
{
    IceUtil::RecMutex::Lock sync(*this);
    return _stats;
}

void
IceInternal::Instance::stats(const StatsPtr& stats)
{
    IceUtil::RecMutex::Lock sync(*this);
    _stats = stats;
}

TraceLevelsPtr
IceInternal::Instance::traceLevels()
{
    // No mutex lock, immutable.
    return _traceLevels;
}

DefaultsAndOverridesPtr
IceInternal::Instance::defaultsAndOverrides()
{
    // No mutex lock, immutable.
    return _defaultsAndOverrides;
}

RouterManagerPtr
IceInternal::Instance::routerManager()
{
    IceUtil::RecMutex::Lock sync(*this);
    return _routerManager;
}

LocatorManagerPtr
IceInternal::Instance::locatorManager()
{
    IceUtil::RecMutex::Lock sync(*this);
    return _locatorManager;
}

ReferenceFactoryPtr
IceInternal::Instance::referenceFactory()
{
    IceUtil::RecMutex::Lock sync(*this);
    return _referenceFactory;
}

ProxyFactoryPtr
IceInternal::Instance::proxyFactory()
{
    IceUtil::RecMutex::Lock sync(*this);
    return _proxyFactory;
}

OutgoingConnectionFactoryPtr
IceInternal::Instance::outgoingConnectionFactory()
{
    IceUtil::RecMutex::Lock sync(*this);
    return _outgoingConnectionFactory;
}

ConnectionMonitorPtr
IceInternal::Instance::connectionMonitor()
{
    IceUtil::RecMutex::Lock sync(*this);
    return _connectionMonitor;
}

ObjectFactoryManagerPtr
IceInternal::Instance::servantFactoryManager()
{
    IceUtil::RecMutex::Lock sync(*this);
    return _servantFactoryManager;
}

UserExceptionFactoryManagerPtr
IceInternal::Instance::userExceptionFactoryManager()
{
    IceUtil::RecMutex::Lock sync(*this);
    return _userExceptionFactoryManager;
}

ObjectAdapterFactoryPtr
IceInternal::Instance::objectAdapterFactory()
{
    IceUtil::RecMutex::Lock sync(*this);
    return _objectAdapterFactory;
}

ThreadPoolPtr
IceInternal::Instance::clientThreadPool()
{
    IceUtil::RecMutex::Lock sync(*this);

    assert(!_destroyed);

    if(!_clientThreadPool) // Lazy initialization.
    {
	int threadNum = _properties->getPropertyAsIntWithDefault("Ice.ThreadPool.Client.Size", 1);
	_clientThreadPool = new ThreadPool(this, threadNum, 0);
    }

    return _clientThreadPool;
}

ThreadPoolPtr
IceInternal::Instance::serverThreadPool()
{
    IceUtil::RecMutex::Lock sync(*this);

    assert(!_destroyed);
    
    if(!_serverThreadPool) // Lazy initialization.
    {
	int threadNum = _properties->getPropertyAsIntWithDefault("Ice.ThreadPool.Server.Size", 10);
	int timeout = _properties->getPropertyAsInt("Ice.ServerIdleTime");
	_serverThreadPool = new ThreadPool(this, threadNum, timeout);
    }

    return _serverThreadPool;
}

EndpointFactoryManagerPtr
IceInternal::Instance::endpointFactoryManager()
{
    IceUtil::RecMutex::Lock sync(*this);
    return _endpointFactoryManager;
}

DynamicLibraryListPtr
IceInternal::Instance::dynamicLibraryList()
{
    IceUtil::RecMutex::Lock sync(*this);
    return _dynamicLibraryList;
}

PluginManagerPtr
IceInternal::Instance::pluginManager()
{
    IceUtil::RecMutex::Lock sync(*this);
    return _pluginManager;
}

IceInternal::Instance::Instance(const CommunicatorPtr& communicator, int& argc, char* argv[],
                                const PropertiesPtr& properties) :
    _destroyed(false),
    _properties(properties)
{
    IceUtil::Mutex::Lock sync(*_globalStateMutex);
    ++_globalStateCounter;

    //
    // Convert command-line options beginning with --Ice. to properties.
    //
    StringSeq args = argsToStringSeq(argc, argv);
    args = properties->parseCommandLineOptions("Ice", args);
    stringSeqToArgs(args, argc, argv);

    try
    {
	__setNoDelete(true);

	if(_globalStateCounter == 1) // Only on first call
	{
	    srand(static_cast<timeval>(IceUtil::Time::now()).tv_usec);
	    
	    if(_properties->getPropertyAsInt("Ice.NullHandleAbort") > 0)
	    {
		IceUtil::nullHandleAbort = true;
	    }

#ifdef _WIN32
	    WORD version = MAKEWORD(1, 1);
	    WSADATA data;
	    if(WSAStartup(version, &data) != 0)
	    {
		_globalStateMutex->unlock();
		SocketException ex(__FILE__, __LINE__);
		ex.error = getSocketErrno();
		throw ex;
	    }
#endif
	    
#ifndef _WIN32
	    struct sigaction action;
	    action.sa_handler = SIG_IGN;
	    sigemptyset(&action.sa_mask);
	    action.sa_flags = 0;
	    sigaction(SIGPIPE, &action, 0);

	    string newUser = _properties->getProperty("Ice.ChangeUser");
	    if(!newUser.empty())
	    {
		struct passwd* pw = getpwnam(newUser.c_str());
		if(!pw)
		{
		    SyscallException ex(__FILE__, __LINE__);
		    ex.error = getSystemErrno();
		    throw ex;
		}
		
		if(setgid(pw->pw_gid) == -1)
		{
		    SyscallException ex(__FILE__, __LINE__);
		    ex.error = getSystemErrno();
		    throw ex;
		}
		
		if(setuid(pw->pw_uid) == -1)
		{
		    SyscallException ex(__FILE__, __LINE__);
		    ex.error = getSystemErrno();
		    throw ex;
		}
	    }
	    
	    if(_properties->getPropertyAsInt("Ice.UseSyslog") > 0)
	    {
		_identForOpenlog = _properties->getProperty("Ice.ProgramName");
		if(_identForOpenlog.empty())
		{
		    _identForOpenlog = "<Unknown Ice Program>";
		}
		openlog(_identForOpenlog.c_str(), LOG_PID, LOG_USER);
	    }
#endif
	}

#ifndef _WIN32
	if(_properties->getPropertyAsInt("Ice.UseSyslog") > 0)
	{
	    _logger = new SysLoggerI;
	}
	else
	{
	    _logger = new LoggerI(_properties->getProperty("Ice.ProgramName"));
	}
#else
	_logger = new LoggerI(_properties->getProperty("Ice.ProgramName"));
#endif

	_stats = 0; // There is no default statistics callback object.

	_traceLevels = new TraceLevels(_properties);

	_defaultsAndOverrides = new DefaultsAndOverrides(_properties);

	_routerManager = new RouterManager;

	_locatorManager = new LocatorManager;

	_referenceFactory = new ReferenceFactory(this);

	_proxyFactory = new ProxyFactory(this);

	_endpointFactoryManager = new EndpointFactoryManager(this);
        EndpointFactoryPtr tcpEndpointFactory = new TcpEndpointFactory(this);
        _endpointFactoryManager->add(tcpEndpointFactory);
        EndpointFactoryPtr udpEndpointFactory = new UdpEndpointFactory(this);
        _endpointFactoryManager->add(udpEndpointFactory);

        _dynamicLibraryList = new DynamicLibraryList;

        _pluginManager = new PluginManagerI(communicator, _dynamicLibraryList);

	_outgoingConnectionFactory = new OutgoingConnectionFactory(this);

	_servantFactoryManager = new ObjectFactoryManager();

	_userExceptionFactoryManager = new UserExceptionFactoryManager();

	_objectAdapterFactory = new ObjectAdapterFactory(this, communicator);

	__setNoDelete(false);
    }
    catch(...)
    {
	destroy();
	__setNoDelete(false);
	--_globalStateCounter;
	throw;
    }
}

IceInternal::Instance::~Instance()
{
    assert(_destroyed);
    assert(!_referenceFactory);
    assert(!_proxyFactory);
    assert(!_outgoingConnectionFactory);
    assert(!_connectionMonitor);
    assert(!_servantFactoryManager);
    assert(!_userExceptionFactoryManager);
    assert(!_objectAdapterFactory);
    assert(!_clientThreadPool);
    assert(!_serverThreadPool);
    assert(!_routerManager);
    assert(!_locatorManager);
    assert(!_endpointFactoryManager);
    assert(!_dynamicLibraryList);
    assert(!_pluginManager);

    if(_globalStateMutex != 0)
    {
	_globalStateMutex->lock();
    }

    assert(_globalStateCounter > 0);
    if(--_globalStateCounter == 0) // Only on last call
    {
#ifdef _WIN32
	WSACleanup();
#endif
	
#ifndef _WIN32
	struct sigaction action;
	action.sa_handler = SIG_DFL;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;
	sigaction(SIGPIPE, &action, 0);
#endif
	
#ifndef _WIN32
	if(!_identForOpenlog.empty())
	{
	    closelog();
	    _identForOpenlog.clear();
	}
#endif
    }
    
    if(_globalStateMutex != 0)
    {
	_globalStateMutex->unlock();
    }
}

void
IceInternal::Instance::finishSetup(int& argc, char* argv[])
{
    //
    // Load plug-ins.
    //
    PluginManagerI* pluginManagerImpl = dynamic_cast<PluginManagerI*>(_pluginManager.get());
    assert(pluginManagerImpl);
    pluginManagerImpl->loadPlugins(argc, argv);

    //
    // Get default router and locator proxies. Don't move this
    // initialization before the plug-in initialization!!! The proxies
    // might depend on endpoint factories to be installed by plug-ins.
    //
    if(!_defaultsAndOverrides->defaultRouter.empty())
    {
	_referenceFactory->setDefaultRouter(
	    RouterPrx::uncheckedCast(_proxyFactory->stringToProxy(_defaultsAndOverrides->defaultRouter)));
    }

    if(!_defaultsAndOverrides->defaultLocator.empty())
    {
	_referenceFactory->setDefaultLocator(
	    LocatorPrx::uncheckedCast(_proxyFactory->stringToProxy(_defaultsAndOverrides->defaultLocator)));
    }

#ifndef _WIN32
    //
    // daemon() must be called after any plug-ins have been
    // installed. For example, an SSL plug-in might want to
    // read a passphrase from standard input.
    //
    // TODO: This is a problem for plug-ins that open files, create
    // threads, etc. Perhaps we need a two-stage plug-in
    // initialization?
    //
    if(_properties->getPropertyAsInt("Ice.Daemon") > 0)
    {
        int noclose = _properties->getPropertyAsInt("Ice.DaemonNoClose");
        int nochdir = _properties->getPropertyAsInt("Ice.DaemonNoChdir");
        
        if(daemon(nochdir, noclose) == -1)
        {
            SyscallException ex(__FILE__, __LINE__);
            ex.error = getSystemErrno();
            throw ex;
        }
    }
#endif

    //
    // Must be done after daemon() is called, since daemon()
    // forks. Does not work together with Ice.Daemon if
    // Ice.DaemonNoClose is not set.
    //
    if(_properties->getPropertyAsInt("Ice.PrintProcessId") > 0)
    {
#ifdef _WIN32
        cout << _getpid() << endl;
#else
        cout << getpid() << endl;
#endif
    }

    //
    // Connection monitor initializations must be done after daemon()
    // is called, since daemon() forks.
    //
    int acmTimeout = _properties->getPropertyAsInt("Ice.ConnectionIdleTime");
    int interval = _properties->getPropertyAsIntWithDefault("Ice.MonitorConnections", acmTimeout);
    if(interval > 0)
    {
	_connectionMonitor = new ConnectionMonitor(this, interval);
    }

    //
    // Thread pool initializations must be done after daemon() is
    // called, since daemon() forks.
    //

    //
    // Thread pool initialization is now lazy initialization in
    // clientThreadPool() and serverThreadPool().
    //
}

void
IceInternal::Instance::destroy()
{
    assert(!_destroyed);

    _objectAdapterFactory->shutdown();
    _objectAdapterFactory->waitForShutdown();
	
    _outgoingConnectionFactory->destroy();
    _outgoingConnectionFactory->waitUntilFinished();

    ThreadPoolPtr serverThreadPool;
    ThreadPoolPtr clientThreadPool;

    {
	IceUtil::RecMutex::Lock sync(*this);

	_objectAdapterFactory = 0;
	_outgoingConnectionFactory = 0;

	if(_connectionMonitor)
	{
	    _connectionMonitor->destroy();
	    _connectionMonitor = 0;
	}

	if(_serverThreadPool)
	{
	    _serverThreadPool->destroy();
	    std::swap(_serverThreadPool, serverThreadPool);
	}
	
	if(_clientThreadPool)
	{
	    _clientThreadPool->destroy();
	    std::swap(_clientThreadPool, clientThreadPool);
	}

	_servantFactoryManager->destroy();
	_servantFactoryManager = 0;
	
	_userExceptionFactoryManager->destroy();
	_userExceptionFactoryManager = 0;
	
	_referenceFactory->destroy();
	_referenceFactory = 0;
	
	// No destroy function defined.
	// _proxyFactory->destroy();
	_proxyFactory = 0;
	
	_routerManager->destroy();
	_routerManager = 0;

	_locatorManager->destroy();
	_locatorManager = 0;

	_endpointFactoryManager->destroy();
	_endpointFactoryManager = 0;

	_pluginManager->destroy();
	_pluginManager = 0;

	// No destroy function defined.
	// _dynamicLibraryList->destroy();
	_dynamicLibraryList = 0;

	_destroyed = true;
    }

    //
    // Join with the thread pool threads outside the synchronization.
    //
    if(clientThreadPool)
    {
	clientThreadPool->joinWithAllThreads();
    }
    if(serverThreadPool)
    {
	serverThreadPool->joinWithAllThreads();
    }
}
