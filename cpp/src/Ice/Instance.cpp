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

#ifdef _WIN32
#   include <Ice/EventLoggerI.h>
#else
#   include <Ice/SysLoggerI.h>
#endif

#ifndef _WIN32
#   include <signal.h>
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

    //
    // Don't throw CommunicatorDestroyedException if destroyed. We
    // need the logger also after destructions.
    //
    return _logger;
}

void
IceInternal::Instance::logger(const LoggerPtr& logger)
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_destroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    _logger = logger;
}

StatsPtr
IceInternal::Instance::stats()
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_destroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    return _stats;
}

void
IceInternal::Instance::stats(const StatsPtr& stats)
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_destroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

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

    if(_destroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    return _routerManager;
}

LocatorManagerPtr
IceInternal::Instance::locatorManager()
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_destroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    return _locatorManager;
}

ReferenceFactoryPtr
IceInternal::Instance::referenceFactory()
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_destroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    return _referenceFactory;
}

ProxyFactoryPtr
IceInternal::Instance::proxyFactory()
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_destroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    return _proxyFactory;
}

OutgoingConnectionFactoryPtr
IceInternal::Instance::outgoingConnectionFactory()
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_destroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    return _outgoingConnectionFactory;
}

ConnectionMonitorPtr
IceInternal::Instance::connectionMonitor()
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_destroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    return _connectionMonitor;
}

ObjectFactoryManagerPtr
IceInternal::Instance::servantFactoryManager()
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_destroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    return _servantFactoryManager;
}

ObjectAdapterFactoryPtr
IceInternal::Instance::objectAdapterFactory()
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_destroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    return _objectAdapterFactory;
}

ThreadPoolPtr
IceInternal::Instance::clientThreadPool()
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_destroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    if(!_clientThreadPool) // Lazy initialization.
    {
// Not necessary anymore, this is now the default for every thread pool
/*
	//
	// Make sure that the client thread pool defaults are correctly
	//
	if(_properties->getProperty("Ice.ThreadPool.Client.Size").empty())
	{
	    _properties->setProperty("Ice.ThreadPool.Client.Size", "1");
	}
	if(_properties->getProperty("Ice.ThreadPool.Client.SizeMax").empty())
	{
	    _properties->setProperty("Ice.ThreadPool.Client.SizeMax", "1");
	}
	if(_properties->getProperty("Ice.ThreadPool.Client.SizeWarn").empty())
	{
	    _properties->setProperty("Ice.ThreadPool.Client.SizeWarn", "0");
	}
*/

	_clientThreadPool = new ThreadPool(this, "Ice.ThreadPool.Client", 0);
    }

    return _clientThreadPool;
}

ThreadPoolPtr
IceInternal::Instance::serverThreadPool()
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_destroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    
    if(!_serverThreadPool) // Lazy initialization.
    {
	int timeout = _properties->getPropertyAsInt("Ice.ServerIdleTime");
	_serverThreadPool = new ThreadPool(this, "Ice.ThreadPool.Server", timeout);
    }

    return _serverThreadPool;
}

EndpointFactoryManagerPtr
IceInternal::Instance::endpointFactoryManager()
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_destroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    return _endpointFactoryManager;
}

DynamicLibraryListPtr
IceInternal::Instance::dynamicLibraryList()
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_destroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    return _dynamicLibraryList;
}

PluginManagerPtr
IceInternal::Instance::pluginManager()
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_destroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    return _pluginManager;
}

size_t
IceInternal::Instance::messageSizeMax() const
{
    // No mutex lock, immutable.
    return _messageSizeMax;
}

void
IceInternal::Instance::flushBatchRequests()
{
    OutgoingConnectionFactoryPtr connectionFactory;
    ObjectAdapterFactoryPtr adapterFactory;

    {
	IceUtil::RecMutex::Lock sync(*this);

	if(_destroyed)
	{
	    throw CommunicatorDestroyedException(__FILE__, __LINE__);
	}

	connectionFactory = _outgoingConnectionFactory;
	adapterFactory = _objectAdapterFactory;
    }

    connectionFactory->flushBatchRequests();
    adapterFactory->flushBatchRequests();
}

IceInternal::Instance::Instance(const CommunicatorPtr& communicator, int& argc, char* argv[],
                                const PropertiesPtr& properties) :
    _destroyed(false),
    _properties(properties)
{
    IceUtil::Mutex::Lock sync(*_globalStateMutex);
    ++_globalStateCounter;

    //
    // Convert command-line options to properties.
    //
    StringSeq args = argsToStringSeq(argc, argv);
    args = _properties->parseIceCommandLineOptions(args);
    stringSeqToArgs(args, argc, argv);

    try
    {
	__setNoDelete(true);

	if(_globalStateCounter == 1) // Only on first call
	{
	    unsigned int seed = static_cast<unsigned int>(IceUtil::Time::now().toMicroSeconds());
	    srand(seed);
	    
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

#ifdef _WIN32
        if(_properties->getPropertyAsInt("Ice.UseEventLog") > 0)
        {
            _logger = new EventLoggerI(_properties->getProperty("Ice.ProgramName"));
        }
        else
        {
            _logger = new LoggerI(_properties->getProperty("Ice.ProgramName"), 
                                  _properties->getPropertyAsInt("Ice.Logger.Timestamp") > 0);
        }
#else
	if(_properties->getPropertyAsInt("Ice.UseSyslog") > 0)
	{
	    _logger = new SysLoggerI;
	}
	else
	{
	    _logger = new LoggerI(_properties->getProperty("Ice.ProgramName"), 
				  _properties->getPropertyAsInt("Ice.Logger.Timestamp") > 0);
	}
#endif

	_stats = 0; // There is no default statistics callback object.

	const_cast<TraceLevelsPtr&>(_traceLevels) = new TraceLevels(_properties);

	const_cast<DefaultsAndOverridesPtr&>(_defaultsAndOverrides) = new DefaultsAndOverrides(_properties);

	static const int defaultMessageSizeMax = 1024;
	Int num = _properties->getPropertyAsIntWithDefault("Ice.MessageSizeMax", defaultMessageSizeMax);
	if(num < 1)
	{
	    _messageSizeMax = defaultMessageSizeMax * 1024; // Ignore stupid values.
	}
	else if(static_cast<size_t>(num) > (size_t)(0x7fffffff / 1024))
	{
	    _messageSizeMax = static_cast<size_t>(0x7fffffff);
	}
	else
	{
	    _messageSizeMax = static_cast<size_t>(num) * 1024; // Property is in kilobytes, _messageSizeMax in bytes.
	}

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

#if !defined(_WIN32) && !defined(__sun) && !defined(__hpux)
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

    if(_objectAdapterFactory)
    {
        _objectAdapterFactory->shutdown();	
    }

    if(_outgoingConnectionFactory)
    {
        _outgoingConnectionFactory->destroy();
    }

    if(_objectAdapterFactory)
    {
        _objectAdapterFactory->waitForShutdown();
    }

    if(_outgoingConnectionFactory)
    {
        _outgoingConnectionFactory->waitUntilFinished();
    }

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

	if(_servantFactoryManager)
	{
	    _servantFactoryManager->destroy();
	    _servantFactoryManager = 0;
	}
	
	if(_referenceFactory)
	{
	    _referenceFactory->destroy();
	    _referenceFactory = 0;
	}
	
	// No destroy function defined.
	// _proxyFactory->destroy();
	_proxyFactory = 0;
	
	if(_routerManager)
	{
	    _routerManager->destroy();
	    _routerManager = 0;
	}

	if(_locatorManager)
	{
	    _locatorManager->destroy();
	    _locatorManager = 0;
	}

	if(_endpointFactoryManager)
	{
	    _endpointFactoryManager->destroy();
	    _endpointFactoryManager = 0;
	}

	if(_pluginManager)
	{
	    _pluginManager->destroy();
	    _pluginManager = 0;
	}

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
