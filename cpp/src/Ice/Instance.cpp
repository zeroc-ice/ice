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
#include <Ice/ProxyFactory.h>
#include <Ice/ThreadPool.h>
#include <Ice/Emitter.h>
#include <Ice/ServantFactoryManager.h>
#include <Ice/ObjectAdapterFactory.h>
#include <Ice/LocalException.h>
#include <Ice/Properties.h>
#include <Ice/LoggerI.h>
#include <Ice/PicklerI.h>

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

JTCMutex* Instance::_globalStateMutex = new JTCMutex;
JTCInitialize* Instance::_globalStateJTC = 0;
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

int Instance::_globalStateCounter = 0;

void IceInternal::incRef(Instance* p) { p->__incRef(); }
void IceInternal::decRef(Instance* p) { p->__decRef(); }

::Ice::CommunicatorPtr
IceInternal::Instance::communicator()
{
    JTCSyncT<JTCMutex> sync(*this);
    return _communicator;
}

::Ice::PropertiesPtr
IceInternal::Instance::properties()
{
    JTCSyncT<JTCMutex> sync(*this);
    return _properties;
}

::Ice::LoggerPtr
IceInternal::Instance::logger()
{
    JTCSyncT<JTCMutex> sync(*this);
    return _logger;
}

void
IceInternal::Instance::logger(const ::Ice::LoggerPtr& logger)
{
    JTCSyncT<JTCMutex> sync(*this);
    _logger = logger;
}

TraceLevelsPtr
IceInternal::Instance::traceLevels()
{
    JTCSyncT<JTCMutex> sync(*this);
    return _traceLevels;
}

ProxyFactoryPtr
IceInternal::Instance::proxyFactory()
{
    JTCSyncT<JTCMutex> sync(*this);
    return _proxyFactory;
}

ThreadPoolPtr
IceInternal::Instance::threadPool()
{
    JTCSyncT<JTCMutex> sync(*this);
    return _threadPool;
}

EmitterFactoryPtr
IceInternal::Instance::emitterFactory()
{
    JTCSyncT<JTCMutex> sync(*this);
    return _emitterFactory;
}

ServantFactoryManagerPtr
IceInternal::Instance::servantFactoryManager()
{
    JTCSyncT<JTCMutex> sync(*this);
    return _servantFactoryManager;
}

ObjectAdapterFactoryPtr
IceInternal::Instance::objectAdapterFactory()
{
    JTCSyncT<JTCMutex> sync(*this);
    return _objectAdapterFactory;
}

PicklerPtr
IceInternal::Instance::pickler()
{
    JTCSyncT<JTCMutex> sync(*this);
    return _pickler;
}

IceInternal::Instance::Instance(const CommunicatorPtr& communicator, const PropertiesPtr& properties)
{
    if (_globalStateMutex != 0)
    {
	_globalStateMutex->lock();
    }

    if (++_globalStateCounter == 1) // Only on first call
    {
	string value;

	// Must be done before "Ice.Daemon" is checked
	value = properties->getProperty("Ice.PrintProcessId");
	if (atoi(value.c_str()) >= 1)
	{
#ifdef WIN32
	    cout << _getpid() << endl;
#else
	    cout << getpid() << endl;
#endif
	}

#ifndef WIN32
	value = properties->getProperty("Ice.Daemon");
	if (atoi(value.c_str()) >= 1)
	{
	    value = properties->getProperty("Ice.DaemonNoClose");
	    int noclose = atoi(value.c_str());

	    value = properties->getProperty("Ice.DaemonNoChdir");
	    int nochdir = atoi(value.c_str());

	    if (daemon(nochdir, noclose) == -1)
	    {
		--_globalStateCounter;
		if (_globalStateMutex != 0)
		{
		    _globalStateMutex->unlock();
		}
		throw SystemException(__FILE__, __LINE__);
	    }
	}
#endif
	
#ifndef WIN32
	value = properties->getProperty("Ice.UseSyslog");
	if (atoi(value.c_str()) >= 1)
	{
	    _identForOpenlog = properties->getProperty("Ice.ProgramName");
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
	    if (_globalStateMutex != 0)
	    {
		_globalStateMutex->unlock();
	    }
	    throw SocketException(__FILE__, __LINE__);
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
	
	if (!JTCInitialize::initialized())
	{
	    _globalStateJTC = new JTCInitialize();
	}
    }
    
    if (_globalStateMutex != 0)
    {
	_globalStateMutex->unlock();
    }

    try
    {
	_communicator = communicator;
	_properties = properties;
#ifndef WIN32
	string value = properties->getProperty("Ice.UseSyslog");
	if (atoi(value.c_str()) >= 1)
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
	_proxyFactory = new ProxyFactory(this);
	_threadPool = new ThreadPool(this);
	_emitterFactory = new EmitterFactory(this);
	_servantFactoryManager = new ServantFactoryManager();
	_objectAdapterFactory = new ObjectAdapterFactory(this);
	_pickler = new PicklerI(this);
    }
    catch(...)
    {
	destroy();
	throw;
    }
}

IceInternal::Instance::~Instance()
{
    assert(!_communicator);
    assert(!_properties);
    assert(!_logger);
    assert(!_traceLevels);
    assert(!_proxyFactory);
    assert(!_threadPool);
    assert(!_emitterFactory);
    assert(!_servantFactoryManager);
    assert(!_objectAdapterFactory);
    assert(!_pickler);

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

	delete _globalStateJTC;
	_globalStateJTC = 0;
    }
    
    if (_globalStateMutex != 0)
    {
	_globalStateMutex->unlock();
    }
}

void
IceInternal::Instance::destroy()
{
    JTCSyncT<JTCMutex> sync(*this);

    //
    // Destroy all contained objects. Then set all references to null,
    // to avoid cyclic object dependencies.
    //

    if(_communicator)
    {
	// Don't destroy the communicator -- the communicator destroys
	// this object, not the other way
	_communicator = 0;
    }

    if(_properties)
    {
	// No destroy function defined
	// _properties->destroy();
	_properties = 0;
    }

    if(_logger)
    {
	_logger->destroy();
	_logger = 0;
    }

    if(_traceLevels)
    {
	// No destroy function defined
	// _traceLevels->destroy();
	_traceLevels = 0;
    }

    if(_proxyFactory)
    {
	// No destroy function defined
	// _proxyFactory->destroy();
	_proxyFactory = 0;
    }

    if(_emitterFactory)
    {
	_emitterFactory->destroy();
	_emitterFactory = 0;
    }

    if(_servantFactoryManager)
    {
	_servantFactoryManager->destroy();
	_servantFactoryManager = 0;
    }

    if(_objectAdapterFactory)
    {
	// No destroy function defined
	// _objectAdapterFactory->destroy();
	_objectAdapterFactory = 0;
    }
    
    if(_pickler)
    {
	// No destroy function defined
	// _pickler->destroy();
	_pickler = 0;
    }

    if(_threadPool)
    {
	_threadPool->waitUntilFinished();
	_threadPool->destroy();
	_threadPool->joinWithAllThreads();
	_threadPool = 0;
    }
}
