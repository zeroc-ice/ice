// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Instance.h>
#include <Ice/TraceLevels.h>
#include <Ice/RouterInfo.h>
#include <Ice/ReferenceFactory.h>
#include <Ice/ProxyFactory.h>
#include <Ice/ThreadPool.h>
#include <Ice/ConnectionFactory.h>
#include <Ice/ObjectFactoryManager.h>
#include <Ice/UserExceptionFactoryManager.h>
#include <Ice/ObjectAdapterFactory.h>
#include <Ice/Exception.h>
#include <Ice/Properties.h>
#include <Ice/LoggerI.h>
#include <Ice/Network.h>

#ifndef WIN32
#   include <Ice/SysLoggerI.h>
#endif

#ifndef WIN32
#   include <csignal>
#   include <syslog.h>
#   include <sys/time.h>
#else
#   include <sys/timeb.h>
#endif

using namespace std;
using namespace Ice;
using namespace IceInternal;

int Instance::_globalStateCounter = 0;
IceUtil::Mutex* Instance::_globalStateMutex = new IceUtil::Mutex;
#ifndef WIN32
string Instance::_identForOpenlog;
#endif

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

CommunicatorPtr
IceInternal::Instance::communicator()
{
    IceUtil::Mutex::Lock sync(*this);
    return _communicator;
}

PropertiesPtr
IceInternal::Instance::properties()
{
    // No mutex lock, immutable.
    return _properties;
}

LoggerPtr
IceInternal::Instance::logger()
{
    IceUtil::Mutex::Lock sync(*this);
    return _logger;
}

void
IceInternal::Instance::logger(const LoggerPtr& logger)
{
    IceUtil::Mutex::Lock sync(*this);
    _logger = logger;
}

TraceLevelsPtr
IceInternal::Instance::traceLevels()
{
    // No mutex lock, immutable.
    return _traceLevels;
}

string
IceInternal::Instance::defaultProtocol()
{
    // No mutex lock, immutable.
    return _defaultProtocol;
}

string
IceInternal::Instance::defaultHost()
{
    // No mutex lock, immutable.
    return _defaultHost;
}

RouterManagerPtr
IceInternal::Instance::routerManager()
{
    IceUtil::Mutex::Lock sync(*this);
    return _routerManager;
}

ReferenceFactoryPtr
IceInternal::Instance::referenceFactory()
{
    IceUtil::Mutex::Lock sync(*this);
    return _referenceFactory;
}

ProxyFactoryPtr
IceInternal::Instance::proxyFactory()
{
    IceUtil::Mutex::Lock sync(*this);
    return _proxyFactory;
}

OutgoingConnectionFactoryPtr
IceInternal::Instance::outgoingConnectionFactory()
{
    IceUtil::Mutex::Lock sync(*this);
    return _outgoingConnectionFactory;
}

ObjectFactoryManagerPtr
IceInternal::Instance::servantFactoryManager()
{
    IceUtil::Mutex::Lock sync(*this);
    return _servantFactoryManager;
}

UserExceptionFactoryManagerPtr
IceInternal::Instance::userExceptionFactoryManager()
{
    IceUtil::Mutex::Lock sync(*this);
    return _userExceptionFactoryManager;
}

ObjectAdapterFactoryPtr
IceInternal::Instance::objectAdapterFactory()
{
    IceUtil::Mutex::Lock sync(*this);
    return _objectAdapterFactory;
}

ThreadPoolPtr
IceInternal::Instance::threadPool()
{
    IceUtil::Mutex::Lock sync(*this);
    return _threadPool;
}

IceInternal::Instance::Instance(const CommunicatorPtr& communicator, const PropertiesPtr& properties) :
    _communicator(communicator),
    _properties(properties)
{
    _globalStateMutex->lock();

    if (++_globalStateCounter == 1) // Only on first call
    {
	// Must be done before "Ice.Daemon" is checked
	if (atoi(_properties->getProperty("Ice.PrintProcessId").c_str()) > 0)
	{
#ifdef WIN32
	    cout << _getpid() << endl;
#else
	    cout << getpid() << endl;
#endif
	}

#ifndef WIN32
	if (atoi(_properties->getProperty("Ice.Daemon").c_str()) > 0)
	{
	    int noclose = atoi(_properties->getProperty("Ice.DaemonNoClose").c_str());
	    int nochdir = atoi(_properties->getProperty("Ice.DaemonNoChdir").c_str());

	    if (daemon(nochdir, noclose) == -1)
	    {
		--_globalStateCounter;
		_globalStateMutex->unlock();
		SystemException ex(__FILE__, __LINE__);
		ex.error = getSystemErrno();
		throw ex;
	    }
	}
#endif
	
#ifndef WIN32
	if (atoi(_properties->getProperty("Ice.UseSyslog").c_str()) > 0)
	{
	    _identForOpenlog = _properties->getProperty("Ice.ProgramName");
	    if (_identForOpenlog.empty())
	    {
		_identForOpenlog = "<Unknown Ice Program>";
	    }
	    openlog(_identForOpenlog.c_str(), LOG_PID, LOG_USER);
	}
#endif

#ifdef WIN32
	WORD version = MAKEWORD(1, 1);
	WSADATA data;
	if (WSAStartup(version, &data) != 0)
	{
	    _globalStateMutex->unlock();
	    SocketException ex(__FILE__, __LINE__);
	    ex.error = getSocketErrno();
	    throw ex;
	}
#endif
	
#ifndef WIN32
	struct sigaction action;
	action.sa_handler = SIG_IGN;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;
	sigaction(SIGPIPE, &action, 0);
#endif

#ifdef WIN32
	struct _timeb tb;
	_ftime(&tb);
	srand(tb.millitm);
#else
	timeval tv;
	gettimeofday(&tv, 0);
	srand(tv.tv_usec);
#endif
	
    }

    _globalStateMutex->unlock();

    try
    {
	__setNoDelete(true);
#ifndef WIN32
	if (atoi(_properties->getProperty("Ice.UseSyslog").c_str()) > 0)
	{
	    _logger = new SysLoggerI;
	}
	else
	{
	    _logger = new LoggerI;
	}
#else
	_logger = new LoggerI;
#endif
	_traceLevels = new TraceLevels(_properties);
	_defaultProtocol = _properties->getProperty("Ice.DefaultProtocol");
	if (_defaultProtocol.empty())
	{
	    _defaultProtocol = "tcp";
	}
	_defaultHost = _properties->getProperty("Ice.DefaultHost");
	if (_defaultHost.empty())
	{
	    _defaultHost = getLocalHost(true);
	}
	_routerManager = new RouterManager;
	_referenceFactory = new ReferenceFactory(this);
	_proxyFactory = new ProxyFactory(this);
	string router = _properties->getProperty("Ice.DefaultRouter");
	if (!router.empty())
	{
	    _referenceFactory->setDefaultRouter(RouterPrx::uncheckedCast(_proxyFactory->stringToProxy(router)));
	}
	_outgoingConnectionFactory = new OutgoingConnectionFactory(this);
	_servantFactoryManager = new ObjectFactoryManager();
	_userExceptionFactoryManager = new UserExceptionFactoryManager();
	_objectAdapterFactory = new ObjectAdapterFactory(this);
	_threadPool = new ThreadPool(this);
	__setNoDelete(false);
    }
    catch(...)
    {
	destroy();
	__setNoDelete(false);
	throw;
    }
}

IceInternal::Instance::~Instance()
{
    assert(!_communicator);
    assert(!_properties);
    assert(!_logger);
    assert(!_traceLevels);
    assert(!_referenceFactory);
    assert(!_proxyFactory);
    assert(!_outgoingConnectionFactory);
    assert(!_servantFactoryManager);
    assert(!_userExceptionFactoryManager);
    assert(!_objectAdapterFactory);
    assert(!_threadPool);
    assert(!_routerManager);

    if (_globalStateMutex != 0)
    {
	_globalStateMutex->lock();
    }

    assert(_globalStateCounter > 0);
    if (--_globalStateCounter == 0) // Only on last call
    {
#ifdef WIN32
	WSACleanup();
#endif
	
#ifndef WIN32
	struct sigaction action;
	action.sa_handler = SIG_DFL;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;
	sigaction(SIGPIPE, &action, 0);
#endif
	
#ifndef WIN32
	if (!_identForOpenlog.empty())
	{
	    closelog();
	    _identForOpenlog.clear();
	}
#endif
    }
    
    if (_globalStateMutex != 0)
    {
	_globalStateMutex->unlock();
    }
}

void
IceInternal::Instance::destroy()
{
    ThreadPoolPtr threadPool;

    {
	IceUtil::Mutex::Lock sync(*this);
	
	//
	// Destroy all contained objects. Then set all references to null,
	// to avoid cyclic object dependencies.
	//
	
	if (_communicator)
	{
	    // Don't destroy the communicator -- the communicator destroys
	    // this object, not the other way.
	    _communicator = 0;
	}
	
	if (_objectAdapterFactory)
	{
	    // Don't shut down the object adapters -- the communicator
	    // must do this before it destroys this object.
	    _objectAdapterFactory = 0;
	}
	
	if (_servantFactoryManager)
	{
	    _servantFactoryManager->destroy();
	    _servantFactoryManager = 0;
	}
	
	if (_userExceptionFactoryManager)
	{
	    _userExceptionFactoryManager->destroy();
	    _userExceptionFactoryManager = 0;
	}
	
	if (_referenceFactory)
	{
	    _referenceFactory->destroy();
	    _referenceFactory = 0;
	}
	
	if (_proxyFactory)
	{
	    // No destroy function defined
	    // _proxyFactory->destroy();
	    _proxyFactory = 0;
	}
	
	if (_outgoingConnectionFactory)
	{
	    _outgoingConnectionFactory->destroy();
	    _outgoingConnectionFactory = 0;
	}
	
	if (_routerManager)
	{
	    _routerManager->destroy();
	    _routerManager = 0;
	}
	
	//
	// We destroy the thread pool outside the thread
	// synchronization.
	//
	threadPool = _threadPool;
	_threadPool = 0;
    }
    
    if (threadPool)
    {
	threadPool->waitUntilFinished();
	threadPool->destroy();
	threadPool->joinWithAllThreads();
    }
}
