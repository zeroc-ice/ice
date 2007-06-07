// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/DisableWarnings.h>
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
#include <IceE/StringUtil.h>

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

static IceUtil::StaticMutex staticMutex = ICE_STATIC_MUTEX_INITIALIZER;
static bool oneOffDone = false;
static int instanceCount = 0;
static bool printProcessIdDone = false;

namespace IceUtil
{

extern bool nullHandleAbort;

}

IceUtil::Shared* IceInternal::upCast(Instance* p) { return p; }

bool
IceInternal::Instance::destroyed() const
{
    IceUtil::RecMutex::Lock sync(*this);
    return _state == StateDestroyed;
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
    IceUtil::RecMutex::Lock sync(*this);

    if(_state == StateDestroyed)
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
    IceUtil::RecMutex::Lock sync(*this);

    if(_state == StateDestroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    return _locatorManager;
}

#endif

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

#ifndef ICEE_PURE_CLIENT
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
#endif

#ifndef ICEE_PURE_BLOCKING_CLIENT
size_t
IceInternal::Instance::threadPerConnectionStackSize() const
{
    // No mutex lock, immutable.
    return _threadPerConnectionStackSize;
}
#endif

EndpointFactoryPtr
IceInternal::Instance::endpointFactory() const
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_state == StateDestroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    return _endpointFactory;
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
	IceUtil::RecMutex::Lock sync(*this);

	if(_state == StateDestroyed)
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

Identity
IceInternal::Instance::stringToIdentity(const string& s) const
{
    Identity ident;

    //
    // Find unescaped separator.
    //
    string::size_type slash = string::npos, pos = 0;
    while((pos = s.find('/', pos)) != string::npos)
    {
        if(pos == 0 || s[pos - 1] != '\\')
        {
            if(slash == string::npos)
            {
                slash = pos;
            }
            else
            {
                //
                // Extra unescaped slash found.
                //
                IdentityParseException ex(__FILE__, __LINE__);
                ex.str = s;
                throw ex;
            }
        }
        pos++;
    }

    if(slash == string::npos)
    {
        if(!IceUtil::unescapeString(s, 0, s.size(), ident.name))
        {
            IdentityParseException ex(__FILE__, __LINE__);
            ex.str = s;
            throw ex;
        }
    }
    else
    {
        if(!IceUtil::unescapeString(s, 0, slash, ident.category))
        {
            IdentityParseException ex(__FILE__, __LINE__);
            ex.str = s;
            throw ex;
        }
        if(slash + 1 < s.size())
        {
            if(!IceUtil::unescapeString(s, slash + 1, s.size(), ident.name))
            {
                IdentityParseException ex(__FILE__, __LINE__);
                ex.str = s;
                throw ex;
            }
        }
    }

#ifdef ICEE_HAS_WSTRING
    if(_initData.stringConverter)
    {
        string tmpString;
        _initData.stringConverter->fromUTF8(reinterpret_cast<const Byte*>(ident.name.data()),
                                            reinterpret_cast<const Byte*>(ident.name.data() + ident.name.size()),
                                            tmpString);
        ident.name = tmpString;

        _initData.stringConverter->fromUTF8(reinterpret_cast<const Byte*>(ident.category.data()),
                                           reinterpret_cast<const Byte*>(ident.category.data() + ident.category.size()),
                                           tmpString);
        ident.category = tmpString;
    }
#endif

    return ident;
}

string
IceInternal::Instance::identityToString(const Identity& ident) const
{
    string name = ident.name;
    string category = ident.category;
#ifdef ICEE_HAS_WSTRING
    if(_initData.stringConverter)
    {
        UTF8BufferI buffer;
        Byte* last = _initData.stringConverter->toUTF8(ident.name.data(), ident.name.data() + ident.name.size(),
                                                       buffer);
        name = string(reinterpret_cast<const char*>(buffer.getBuffer()), last - buffer.getBuffer());

        buffer.reset();
        last = _initData.stringConverter->toUTF8(ident.category.data(), ident.category.data() + ident.category.size(),
                                                 buffer);
        category = string(reinterpret_cast<const char*>(buffer.getBuffer()), last - buffer.getBuffer());
    }
#endif
    if(category.empty())
    {
        return IceUtil::escapeString(name, "/");
    }
    else
    {
        return IceUtil::escapeString(category, "/") + '/' + IceUtil::escapeString(name, "/");
    }
}

IceInternal::Instance::Instance(const CommunicatorPtr& communicator, const InitializationData& initData) :
    _state(StateActive),
    _initData(initData),
    _messageSizeMax(0)
#ifndef ICEE_PURE_BLOCKING_CLIENT
    , _threadPerConnectionStackSize(0)
#endif
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

	    string stdOutFilename = _initData.properties->getProperty("Ice.StdOut");
	    string stdErrFilename = _initData.properties->getProperty("Ice.StdErr");
	    
	    if(stdOutFilename != "")
	    {
	    	FILE * file;
#ifdef _WIN32_WCE
		wchar_t* wtext = new wchar_t[sizeof(wchar_t) * stdOutFilename.length()];
		mbstowcs(wtext, stdOutFilename.c_str(), stdOutFilename.length());
		file = _wfreopen(wtext, L"a", stdout);
		delete wtext;
#else
		file = freopen(stdOutFilename.c_str(), "a", stdout);
#endif
		if(file == 0)
		{
		    SyscallException ex(__FILE__, __LINE__);
		    ex.error = getSystemErrno();
		    throw ex;
		}
	    }
	    
	    if(stdErrFilename != "")
	    {
	    	FILE* file;
#ifdef _WIN32_WCE
		wchar_t* wtext = new wchar_t[sizeof(wchar_t) * stdErrFilename.length()];
		mbstowcs(wtext, stdErrFilename.c_str(), stdErrFilename.length());
		file = _wfreopen(wtext, L"a", stderr);
		delete wtext;
#else
		file = freopen(stdErrFilename.c_str(), "a", stderr);
#endif
		if(file == 0)
		{
		    SyscallException ex(__FILE__, __LINE__);
		    ex.error = getSystemErrno();
		    throw ex;
		}
	    }
	    
	    unsigned int seed = static_cast<unsigned int>(IceUtil::Time::now().toMicroSeconds());
	    srand(seed);
#ifndef _WIN32
	    srand48(seed);
#endif
	    
	    if(_initData.properties->getPropertyAsInt("Ice.NullHandleAbort") > 0)
	    {
		IceUtil::nullHandleAbort = true;
	    }
	    
#ifndef _WIN32
	    string newUser = _initData.properties->getProperty("Ice.ChangeUser");
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
	

	if(!_initData.logger)
	{
	    _initData.logger = new LoggerI(_initData.properties->getProperty("Ice.ProgramName"));
	}

	const_cast<TraceLevelsPtr&>(_traceLevels) = new TraceLevels(_initData.properties);

	const_cast<DefaultsAndOverridesPtr&>(_defaultsAndOverrides) = new DefaultsAndOverrides(_initData.properties);

	{
	    static const int defaultMessageSizeMax = 1024;
	    Int num = _initData.properties->getPropertyAsIntWithDefault("Ice.MessageSizeMax", defaultMessageSizeMax);
	    if(num < 1)
	    {
		const_cast<size_t&>(_messageSizeMax) = defaultMessageSizeMax * 1024; // Ignore non-sensical values.
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

#ifndef ICEE_PURE_BLOCKING_CLIENT
	{
	    Int stackSize = _initData.properties->getPropertyAsInt("Ice.ThreadPerConnection.StackSize");
	    if(stackSize < 0)
	    {
		stackSize = 0;
	    }
	    const_cast<size_t&>(_threadPerConnectionStackSize) = static_cast<size_t>(stackSize);
	}
#endif

#ifdef ICEE_HAS_ROUTER
	_routerManager = new RouterManager;
#endif

#ifdef ICEE_HAS_LOCATOR
	_locatorManager = new LocatorManager;
#endif

	_referenceFactory = new ReferenceFactory(this, communicator);

	_proxyFactory = new ProxyFactory(this);

        _endpointFactory = new EndpointFactory(this);

	_outgoingConnectionFactory = new OutgoingConnectionFactory(this);

#ifndef ICEE_PURE_CLIENT
	_objectAdapterFactory = new ObjectAdapterFactory(this, communicator);
#endif

#ifdef ICEE_HAS_WSTRING
        if(!_initData.wstringConverter)
        {
            _initData.wstringConverter = new UnicodeWstringConverter();
        }
#endif

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

#ifndef _WIN32_WCE
    //
    // Show process id if requested (but only once).
    //
    bool printProcessId = false;
    if(!printProcessIdDone && _initData.properties->getPropertyAsInt("Ice.PrintProcessId") > 0)
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
    {
	IceUtil::RecMutex::Lock sync(*this);
	
	//
	// If the _state is not StateActive then the instance is
	// either being destroyed, or has already been destroyed.
	//
	if(_state != StateActive)
	{
	    return;
	}

	//
	// We cannot set state to StateDestroyed otherwise instance
	// methods called during the destroy process (such as
	// outgoingConnectionFactory() from
	// ObjectAdapterI::deactivate() will cause an exception.
	//
	_state = StateDestroyInProgress;
    }

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
        _objectAdapterFactory->destroy();
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
	IceUtil::RecMutex::Lock sync(*this);

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

	_state = StateDestroyed;
    }
}

#ifdef ICEE_HAS_WSTRING
IceInternal::UTF8BufferI::UTF8BufferI() :
    _buffer(0),
    _offset(0)
{
}

IceInternal::UTF8BufferI::~UTF8BufferI()
{
    free(_buffer);
}

Byte*
IceInternal::UTF8BufferI::getMoreBytes(size_t howMany, Byte* firstUnused)
{
    if(_buffer == 0)
    {
        _buffer = (Byte*)malloc(howMany);
    }
    else
    {
        assert(firstUnused != 0);
        _offset = firstUnused - _buffer;
        _buffer = (Byte*)realloc(_buffer, _offset + howMany);
    }

    return _buffer + _offset;
}

Byte*
IceInternal::UTF8BufferI::getBuffer()
{
    return _buffer;
}

void
IceInternal::UTF8BufferI::reset()
{
    free(_buffer);
    _buffer = 0;
    _offset = 0;
}
#endif
