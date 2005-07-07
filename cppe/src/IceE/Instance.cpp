// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/Instance.h>
#include <IceE/TraceLevels.h>
#include <IceE/DefaultsAndOverrides.h>
#ifdef ICEE_HAS_ROUTER
#    include <IceE/RouterInfo.h>
#    include <IceE/Router.h>
#endif
#ifdef ICEE_HAS_LOCATOR
#    include <IceE/LocatorInfo.h>
#    include <IceE/Locator.h>
#endif
#include <IceE/ReferenceFactory.h>
#include <IceE/ProxyFactory.h>
#include <IceE/OutgoingConnectionFactory.h>
#include <IceE/LocalException.h>
#include <IceE/Properties.h>
#include <IceE/LoggerI.h>
#include <IceE/EndpointFactory.h>
#ifndef ICEE_PURE_CLIENT
#    include <IceE/ObjectAdapterFactory.h>
#endif

#include <IceE/StaticMutex.h>

#include <stdio.h>

#ifdef _WIN32
#   include <winsock2.h>
#else
#   include <signal.h>
#   include <pwd.h>
#   include <sys/types.h>
#endif

using namespace std;
using namespace Ice;
using namespace IceInternal;

static Ice::StaticMutex staticMutex = ICEE_STATIC_MUTEX_INITIALIZER;
static bool oneOffDone = false;
static int instanceCount = 0;
static bool printProcessIdDone = false;

namespace Ice
{

extern bool ICEE_API nullHandleAbort;

}

void IceInternal::incRef(Instance* p) { p->__incRef(); }
void IceInternal::decRef(Instance* p) { p->__decRef(); }

PropertiesPtr
IceInternal::Instance::properties() const
{
    // No mutex lock, immutable.
    return _properties;
}

LoggerPtr
IceInternal::Instance::logger() const
{
    Ice::RecMutex::Lock sync(*this);

    //
    // Don't throw CommunicatorDestroyedException if destroyed. We
    // need the logger also after destructions.
    //
    return _logger;
}

void
IceInternal::Instance::logger(const LoggerPtr& logger)
{
    Ice::RecMutex::Lock sync(*this);

    if(_destroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    _logger = logger;
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

#ifdef ICEE_HAS_ROUTER

RouterManagerPtr
IceInternal::Instance::routerManager() const
{
    Ice::RecMutex::Lock sync(*this);

    if(_destroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    return _routerManager;
}

#endif

#ifdef ICEE_HAS_LOCATOR

LocatorManagerPtr
IceInternal::Instance::locatorManager() const
{
    Ice::RecMutex::Lock sync(*this);

    if(_destroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    return _locatorManager;
}

#endif

ReferenceFactoryPtr
IceInternal::Instance::referenceFactory() const
{
    Ice::RecMutex::Lock sync(*this);

    if(_destroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    return _referenceFactory;
}

ProxyFactoryPtr
IceInternal::Instance::proxyFactory() const
{
    Ice::RecMutex::Lock sync(*this);

    if(_destroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    return _proxyFactory;
}

OutgoingConnectionFactoryPtr
IceInternal::Instance::outgoingConnectionFactory() const
{
    Ice::RecMutex::Lock sync(*this);

    if(_destroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    return _outgoingConnectionFactory;
}

#ifndef ICEE_PURE_CLIENT
ObjectAdapterFactoryPtr
IceInternal::Instance::objectAdapterFactory() const
{
    Ice::RecMutex::Lock sync(*this);

    if(_destroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    return _objectAdapterFactory;
}
#endif

size_t
IceInternal::Instance::threadPerConnectionStackSize() const
{
    // No mutex lock, immutable.
    return _threadPerConnectionStackSize;
}

EndpointFactoryPtr
IceInternal::Instance::endpointFactory() const
{
    Ice::RecMutex::Lock sync(*this);

    if(_destroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    return _endpointFactory;
}

size_t
IceInternal::Instance::messageSizeMax() const
{
    // No mutex lock, immutable.
    return _messageSizeMax;
}

#ifdef ICEE_HAS_BATCH
void
IceInternal::Instance::flushBatchRequests()
{

    OutgoingConnectionFactoryPtr connectionFactory;
#ifndef ICEE_PURE_CLIENT
    ObjectAdapterFactoryPtr adapterFactory;
#endif

    {
	Ice::RecMutex::Lock sync(*this);

	if(_destroyed)
	{
	    throw CommunicatorDestroyedException(__FILE__, __LINE__);
	}

	connectionFactory = _outgoingConnectionFactory;
#ifndef ICEE_PURE_CLIENT
	adapterFactory = _objectAdapterFactory;
#endif
    }

    connectionFactory->flushBatchRequests();
#ifndef ICEE_PURE_CLIENT
    adapterFactory->flushBatchRequests();
#endif

}
#endif

void
IceInternal::Instance::setDefaultContext(const Context& ctx)
{
    _defaultContext = ctx;
}

const Context&
IceInternal::Instance::getDefaultContext() const
{
    return _defaultContext;
}

IceInternal::Instance::Instance(const CommunicatorPtr& communicator, const PropertiesPtr& properties) :
    _destroyed(false),
    _properties(properties),
    _messageSizeMax(0),
    _threadPerConnectionStackSize(0)
{
    try
    {
	__setNoDelete(true);

	Ice::StaticMutex::Lock sync(staticMutex);
	instanceCount++;

	if(!oneOffDone)
	{
	    //
	    // StdOut and StdErr redirection
	    //

#ifndef _WIN32_WCE // XXX:DGB How do redirect on WinCE?
	    string stdOutFilename = _properties->getProperty("IceE.StdOut");
	    string stdErrFilename = _properties->getProperty("IceE.StdErr");
	    
	    if(stdOutFilename != "")
	    {
		FILE* file = freopen(stdOutFilename.c_str(), "a", stdout);
		if(file == 0)
		{
		    SyscallException ex(__FILE__, __LINE__);
		    ex.error = getSystemErrno();
		    throw ex;
		}
	    }
	    
	    if(stdErrFilename != "")
	    {
		FILE* file = freopen(stdErrFilename.c_str(), "a", stderr);
		if(file == 0)
		{
		    SyscallException ex(__FILE__, __LINE__);
		    ex.error = getSystemErrno();
		    throw ex;
		}
	    }
#endif
	    
	    unsigned int seed = static_cast<unsigned int>(Ice::Time::now().toMicroSeconds());
	    srand(seed);
#ifndef _WIN32
	    srand48(seed);
#endif
	    
	    if(_properties->getPropertyAsInt("IceE.NullHandleAbort") > 0)
	    {
		Ice::nullHandleAbort = true;
	    }
	    
#ifndef _WIN32
	    string newUser = _properties->getProperty("IceE.ChangeUser");
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
		ex.error = WSAGetLastError();
		throw ex;
	    }
#endif
	    
#ifndef _WIN32
	    struct sigaction action;
	    action.sa_handler = SIG_IGN;
	    sigemptyset(&action.sa_mask);
	    action.sa_flags = 0;
	    sigaction(SIGPIPE, &action, 0);
#endif
	}
	
	sync.release();
	

	_logger = new LoggerI(_properties->getProperty("IceE.ProgramName"));

	const_cast<TraceLevelsPtr&>(_traceLevels) = new TraceLevels(_properties);

	const_cast<DefaultsAndOverridesPtr&>(_defaultsAndOverrides) = new DefaultsAndOverrides(_properties);

	{
	    static const int defaultMessageSizeMax = 1024;
	    Int num = _properties->getPropertyAsIntWithDefault("IceE.MessageSizeMax", defaultMessageSizeMax);
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
	    Int stackSize = _properties->getPropertyAsInt("IceE.ThreadPerConnection.StackSize");
	    if(stackSize < 0)
	    {
		stackSize = 0;
	    }
	    const_cast<size_t&>(_threadPerConnectionStackSize) = static_cast<size_t>(stackSize);
	}

#ifdef ICEE_HAS_ROUTER
	_routerManager = new RouterManager;
#endif

#ifdef ICEE_HAS_LOCATOR
	_locatorManager = new LocatorManager;
#endif

	_referenceFactory = new ReferenceFactory(this);

	_proxyFactory = new ProxyFactory(this);

        _endpointFactory = new EndpointFactory(this);

	_outgoingConnectionFactory = new OutgoingConnectionFactory(this);

#ifndef ICEE_PURE_CLIENT
	_objectAdapterFactory = new ObjectAdapterFactory(this, communicator);
#endif

	__setNoDelete(false);
    }
    catch(...)
    {
	{
	    Ice::StaticMutex::Lock sync(staticMutex);
	    --instanceCount;
	}
	destroy();
	__setNoDelete(false);
	throw;
    }
}

IceInternal::Instance::~Instance()
{
    assert(_destroyed);
    assert(!_referenceFactory);
    assert(!_proxyFactory);
    assert(!_outgoingConnectionFactory);
#ifndef ICEE_PURE_CLIENT
    assert(!_objectAdapterFactory);
#endif
#ifdef ICEE_HAS_ROUTER
    assert(!_routerManager);
#endif
#ifdef ICEE_HAS_LOCATOR
    assert(!_locatorManager);
#endif
    assert(!_endpointFactory);

    Ice::StaticMutex::Lock sync(staticMutex);
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
#endif
    }
}

void
IceInternal::Instance::finishSetup(int& argc, char* argv[])
{
    //
    // Get default router and locator proxies. Don't move this
    // initialization before the plug-in initialization!!! The proxies
    // might depend on endpoint factories to be installed by plug-ins.
    //
#ifdef ICEE_HAS_ROUTER
    if(!_defaultsAndOverrides->defaultRouter.empty())
    {
	_referenceFactory->setDefaultRouter(
	    RouterPrx::uncheckedCast(_proxyFactory->stringToProxy(_defaultsAndOverrides->defaultRouter)));
    }
#endif

#ifdef ICEE_HAS_LOCATOR
    if(!_defaultsAndOverrides->defaultLocator.empty())
    {
	_referenceFactory->setDefaultLocator(
	    LocatorPrx::uncheckedCast(_proxyFactory->stringToProxy(_defaultsAndOverrides->defaultLocator)));
    }
#endif

    //
    // Show process id if requested (but only once).
    //
    bool printProcessId = false;
    if(!printProcessIdDone && _properties->getPropertyAsInt("IceE.PrintProcessId") > 0)
    {
	//
	// Safe double-check locking (no dependent variable!)
	// 
	Ice::StaticMutex::Lock sync(staticMutex);
	printProcessId = !printProcessIdDone;
	
	//
	// We anticipate: we want to print it once, and we don't care when.
	//
	printProcessIdDone = true;
    }

#ifndef _WIN32_WCE
    if(printProcessId)
    {
#ifdef _WIN32
	printf("%d\n", _getpid());
#else
	printf("%d\n", getpid()); 
#endif
	fflush(stdout);
    }
#endif
}

void
IceInternal::Instance::destroy()
{
    assert(!_destroyed);

#ifndef ICEE_PURE_CLIENT
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
#else
    if(_outgoingConnectionFactory)
    {
        _outgoingConnectionFactory->destroy();
        _outgoingConnectionFactory->waitUntilFinished();
    }
#endif

    {
	Ice::RecMutex::Lock sync(*this);

#ifndef ICEE_PURE_CLIENT
	_objectAdapterFactory = 0;
#endif
	_outgoingConnectionFactory = 0;

	if(_referenceFactory)
	{
	    _referenceFactory->destroy();
	    _referenceFactory = 0;
	}
	
	// No destroy function defined.
	// _proxyFactory->destroy();
	_proxyFactory = 0;
	
#ifdef ICEE_HAS_ROUTER
	if(_routerManager)
	{
	    _routerManager->destroy();
	    _routerManager = 0;
	}
#endif

#ifdef ICEE_HAS_LOCATOR
	if(_locatorManager)
	{
	    _locatorManager->destroy();
	    _locatorManager = 0;
	}
#endif

	if(_endpointFactory)
	{
	    _endpointFactory->destroy();
	    _endpointFactory = 0;
	}

	_destroyed = true;
    }
}
