// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Instance.h>
#include <Ice/TraceLevels.h>
#include <Ice/DefaultsAndOverrides.h>
#include <Ice/RouterInfo.h>
#include <Ice/Router.h>
#include <Ice/LocatorInfo.h>
#include <Ice/Locator.h>
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

#include <stdio.h>

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

static IceUtil::StaticMutex staticMutex = ICE_STATIC_MUTEX_INITIALIZER;
static bool oneOffDone = false;
static int instanceCount = 0;
static bool printProcessIdDone = false;
static string identForOpenlog;

namespace IceUtil
{

extern bool ICE_UTIL_API nullHandleAbort;

}

void IceInternal::incRef(Instance* p) { p->__incRef(); }
void IceInternal::decRef(Instance* p) { p->__decRef(); }

bool
IceInternal::Instance::destroyed() const
{
    IceUtil::RecMutex::Lock sync(*this);
    return _state == StateDestroyed;
}

CommunicatorPtr
IceInternal::Instance::communicator() const
{
    return _communicator;
}

PropertiesPtr
IceInternal::Instance::properties() const
{
    //
    // No check for destruction. It must be possible to access the
    // properties after destruction.
    //
    // No mutex lock, immutable.
    //
    return _properties;
}

LoggerPtr
IceInternal::Instance::logger() const
{
    //
    // No check for destruction. It must be possible to access the
    // logger after destruction.
    //
    IceUtil::RecMutex::Lock sync(*this);
    return _logger;
}

void
IceInternal::Instance::logger(const LoggerPtr& logger)
{
    //
    // No check for destruction. It must be possible to set the logger
    // after destruction (needed by logger plugins for example to
    // unset the logger).
    //
    IceUtil::RecMutex::Lock sync(*this);
    _logger = logger;
}

StatsPtr
IceInternal::Instance::stats() const
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_state == StateDestroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    return _stats;
}

void
IceInternal::Instance::stats(const StatsPtr& stats)
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_state == StateDestroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    _stats = stats;
}

TraceLevelsPtr
IceInternal::Instance::traceLevels() const
{
    // No mutex lock, immutable.
    return _traceLevels;
}

DefaultsAndOverridesPtr
IceInternal::Instance::defaultsAndOverrides() const
{
    // No mutex lock, immutable.
    return _defaultsAndOverrides;
}

RouterManagerPtr
IceInternal::Instance::routerManager() const
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_state == StateDestroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    return _routerManager;
}

LocatorManagerPtr
IceInternal::Instance::locatorManager() const
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_state == StateDestroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    return _locatorManager;
}

ReferenceFactoryPtr
IceInternal::Instance::referenceFactory() const
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_state == StateDestroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    return _referenceFactory;
}

ProxyFactoryPtr
IceInternal::Instance::proxyFactory() const
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_state == StateDestroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    return _proxyFactory;
}

OutgoingConnectionFactoryPtr
IceInternal::Instance::outgoingConnectionFactory() const
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_state == StateDestroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    return _outgoingConnectionFactory;
}

ConnectionMonitorPtr
IceInternal::Instance::connectionMonitor() const
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_state == StateDestroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    return _connectionMonitor;
}

ObjectFactoryManagerPtr
IceInternal::Instance::servantFactoryManager() const
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_state == StateDestroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    return _servantFactoryManager;
}

ObjectAdapterFactoryPtr
IceInternal::Instance::objectAdapterFactory() const
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_state == StateDestroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    return _objectAdapterFactory;
}

ThreadPoolPtr
IceInternal::Instance::clientThreadPool()
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_state == StateDestroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    if(!_clientThreadPool) // Lazy initialization.
    {
	_clientThreadPool = new ThreadPool(this, "Ice.ThreadPool.Client", 0);
    }

    return _clientThreadPool;
}

ThreadPoolPtr
IceInternal::Instance::serverThreadPool()
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_state == StateDestroyed)
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

bool
IceInternal::Instance::threadPerConnection() const
{
    // No mutex lock, immutable.
    return _threadPerConnection;
}

size_t
IceInternal::Instance::threadPerConnectionStackSize() const
{
    // No mutex lock, immutable.
    return _threadPerConnectionStackSize;
}

EndpointFactoryManagerPtr
IceInternal::Instance::endpointFactoryManager() const
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_state == StateDestroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    return _endpointFactoryManager;
}

DynamicLibraryListPtr
IceInternal::Instance::dynamicLibraryList() const
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_state == StateDestroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    return _dynamicLibraryList;
}

PluginManagerPtr
IceInternal::Instance::pluginManager() const
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_state == StateDestroyed)
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

int
IceInternal::Instance::clientACM() const
{
    // No mutex lock, immutable.
    return _clientACM;
}

int
IceInternal::Instance::serverACM() const
{
    // No mutex lock, immutable.
    return _serverACM;
}

void
IceInternal::Instance::flushBatchRequests()
{
    OutgoingConnectionFactoryPtr connectionFactory;
    ObjectAdapterFactoryPtr adapterFactory;

    {
	IceUtil::RecMutex::Lock sync(*this);

	if(_state == StateDestroyed)
	{
	    throw CommunicatorDestroyedException(__FILE__, __LINE__);
	}

	connectionFactory = _outgoingConnectionFactory;
	adapterFactory = _objectAdapterFactory;
    }

    connectionFactory->flushBatchRequests();
    adapterFactory->flushBatchRequests();
}

void
IceInternal::Instance::setDefaultContext(const Context& ctx)
{
    IceUtil::RecMutex::Lock sync(*this);
    
    if(_state == StateDestroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    _defaultContext = ctx;
}

const Context&
IceInternal::Instance::getDefaultContext() const
{
    IceUtil::RecMutex::Lock sync(*this);
    
    if(_state == StateDestroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    return _defaultContext;
}


IceInternal::Instance::Instance(const CommunicatorPtr& communicator, const PropertiesPtr& properties) :
    _communicator(communicator.get()),
    _state(StateActive),
    _properties(properties),
    _messageSizeMax(0),
    _clientACM(0),
    _serverACM(0),
    _threadPerConnection(false),
    _threadPerConnectionStackSize(0)
{
    try
    {
	__setNoDelete(true);

	IceUtil::StaticMutex::Lock sync(staticMutex);
	instanceCount++;

	if(!oneOffDone)
	{
	    //
	    // StdOut and StdErr redirection
	    //
	    string stdOutFilename = _properties->getProperty("Ice.StdOut");
	    string stdErrFilename = _properties->getProperty("Ice.StdErr");
	    
	    if(stdOutFilename != "")
	    {
		FILE* file = freopen(stdOutFilename.c_str(), "a", stdout);
		if(file == 0)
		{
		    FileException ex(__FILE__, __LINE__);
		    ex.path = stdOutFilename;
		    ex.error = getSystemErrno();
		    throw ex;
		}
	    }
	    
	    if(stdErrFilename != "")
	    {
		FILE* file = freopen(stdErrFilename.c_str(), "a", stderr);
		if(file == 0)
		{
		    FileException ex(__FILE__, __LINE__);
		    ex.path = stdErrFilename;
		    ex.error = getSystemErrno();
		    throw ex;
		}
	    }
	    
	    unsigned int seed = static_cast<unsigned int>(IceUtil::Time::now().toMicroSeconds());
	    srand(seed);
#ifndef _WIN32
	    srand48(seed);
#endif
	    
	    if(_properties->getPropertyAsInt("Ice.NullHandleAbort") > 0)
	    {
		IceUtil::nullHandleAbort = true;
	    }
	    
#ifndef _WIN32
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
#endif
	    oneOffDone = true;
	}   
	
	if(instanceCount == 1)
	{	 	    
	    
#ifdef _WIN32
	    WORD version = MAKEWORD(1, 1);
	    WSADATA data;
	    if(WSAStartup(version, &data) != 0)
	    {
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
	    
	    if(_properties->getPropertyAsInt("Ice.UseSyslog") > 0)
	    {
		identForOpenlog = _properties->getProperty("Ice.ProgramName");
		if(identForOpenlog.empty())
		{
		    identForOpenlog = "<Unknown Ice Program>";
		}
		openlog(identForOpenlog.c_str(), LOG_PID, LOG_USER);
	    }
#endif
	}
	
	sync.release();
	

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

	{
	    static const int defaultMessageSizeMax = 1024;
	    Int num = _properties->getPropertyAsIntWithDefault("Ice.MessageSizeMax", defaultMessageSizeMax);
	    if(num < 1)
	    {
		const_cast<size_t&>(_messageSizeMax) = defaultMessageSizeMax * 1024; // Ignore stupid values.
	    }
	    else if(static_cast<size_t>(num) > (size_t)(0x7fffffff / 1024))
	    {
		const_cast<size_t&>(_messageSizeMax) = static_cast<size_t>(0x7fffffff);
	    }
	    else
	    {
		// Property is in kilobytes, _messageSizeMax in bytes.
		const_cast<size_t&>(_messageSizeMax) = static_cast<size_t>(num) * 1024;
	    }
	}

	{
	    Int clientACMDefault = 60; // Client ACM enabled by default.
	    Int serverACMDefault = 0; // Server ACM disabled by default.
	    
	    //
	    // Legacy: If Ice.ConnectionIdleTime is set, we use it as
	    // default value for both the client- and server-side ACM.
	    //
	    if(!_properties->getProperty("Ice.ConnectionIdleTime").empty())
	    {
		Int num = _properties->getPropertyAsInt("Ice.ConnectionIdleTime");
		clientACMDefault = num;
		serverACMDefault = num;
	    }
	    
	    const_cast<Int&>(_clientACM) = _properties->getPropertyAsIntWithDefault("Ice.ACM.Client",
										    clientACMDefault);
	    const_cast<Int&>(_serverACM) = _properties->getPropertyAsIntWithDefault("Ice.ACM.Server",
										    serverACMDefault);
	}

	const_cast<bool&>(_threadPerConnection) = _properties->getPropertyAsInt("Ice.ThreadPerConnection") > 0;

	{
	    Int stackSize = _properties->getPropertyAsInt("Ice.ThreadPerConnection.StackSize");
	    if(stackSize < 0)
	    {
		stackSize = 0;
	    }
	    const_cast<size_t&>(_threadPerConnectionStackSize) = static_cast<size_t>(stackSize);
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
	{
	    IceUtil::StaticMutex::Lock sync(staticMutex);
	    --instanceCount;
	}
	destroy();
	__setNoDelete(false);
	throw;
    }
}

IceInternal::Instance::~Instance()
{
    assert(_state == StateDestroyed);
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

    IceUtil::StaticMutex::Lock sync(staticMutex);
    if(--instanceCount == 0)
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
	
	if(!identForOpenlog.empty())
	{
	    closelog();
	    identForOpenlog.clear();
	}
#endif
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

    //
    // Show process id if requested (but only once).
    //
    bool printProcessId = false;
    if(!printProcessIdDone && _properties->getPropertyAsInt("Ice.PrintProcessId") > 0)
    {
	//
	// Safe double-check locking (no dependent variable!)
	// 
	IceUtil::StaticMutex::Lock sync(staticMutex);
	printProcessId = !printProcessIdDone;
	
	//
	// We anticipate: we want to print it once, and we don't care when.
	//
	printProcessIdDone = true;
    }

    if(printProcessId)
    {
#ifdef _WIN32
	cout << _getpid() << endl;
#else
	cout << getpid() << endl;
#endif
    }

    //
    // Start connection monitor if necessary.
    //
    Int interval = 0;
    if(_clientACM > 0 && _serverACM > 0)
    {
	interval = min(_clientACM, _serverACM);
    }
    else if(_clientACM > 0)
    {
	interval = _clientACM;
    }
    else if(_serverACM > 0)
    {
	interval = _serverACM;
    }
    interval = _properties->getPropertyAsIntWithDefault("Ice.MonitorConnections", interval);
    if(interval > 0)
    {
	_connectionMonitor = new ConnectionMonitor(this, interval);
    }

    //
    // Thread pool initialization is now lazy initialization in
    // clientThreadPool() and serverThreadPool().
    //
}

bool
IceInternal::Instance::destroy()
{
    {
	IceUtil::RecMutex::Lock sync(*this);
	
	//
	// If the _state is not StateActive then the instance is
	// either being destroyed, or has already been destroyed.
	//
	if(_state != StateActive)
	{
	    return false;
	}

	//
	// We cannot set state to StateDestroyed otherwise instance
	// methods called during the destroy process (such as
	// outgoingConnectionFactory() from
	// ObjectAdapterI::deactivate() will cause an exception.
	//
	_state = StateDestroyInProgress;
    }

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

	_state = StateDestroyed;
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
    return true;
}
