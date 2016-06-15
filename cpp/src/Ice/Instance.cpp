// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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
#include <Ice/ObjectFactoryManager.h>
#include <Ice/LocalException.h>
#include <Ice/ObjectAdapterFactory.h>
#include <Ice/Exception.h>
#include <Ice/PropertiesI.h>
#include <Ice/PropertiesAdminI.h>
#include <Ice/LoggerI.h>
#include <Ice/NetworkProxy.h>
#include <Ice/EndpointFactoryManager.h>
#include <Ice/IPEndpointI.h> // For EndpointHostResolver
#include <Ice/WSEndpoint.h>
#include <Ice/RequestHandlerFactory.h>
#include <Ice/RetryQueue.h>
#include <Ice/DynamicLibrary.h>
#include <Ice/PluginManagerI.h>
#include <Ice/Initialize.h>
#include <Ice/LoggerUtil.h>
#include <Ice/PropertiesI.h>
#include <Ice/Communicator.h>
#include <Ice/InstrumentationI.h>
#include <Ice/ProtocolInstance.h>
#include <Ice/LoggerAdminI.h>
#include <Ice/RegisterPluginsInit.h>
#include <Ice/ObserverHelper.h>

#include <IceUtil/StringUtil.h>
#include <IceUtil/UUID.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/MutexPtrLock.h>
#include <IceUtil/Atomic.h>

#include <stdio.h>
#include <list>

#ifndef _WIN32
#   include <Ice/SysLoggerI.h>

#   include <signal.h>
#   include <syslog.h>
#   include <pwd.h>
#   include <sys/types.h>
#endif

#if defined(__linux) || defined(__sun) || defined(_AIX) || defined(__GLIBC__)
#   include <grp.h> // for initgroups
#endif

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace IceUtilInternal
{

extern bool ICE_UTIL_API nullHandleAbort;
extern bool ICE_UTIL_API printStackTraces;

};

namespace
{

IceUtil::Mutex* staticMutex = 0;
bool oneOfDone = false;
std::list<IceInternal::Instance*>* instanceList = 0;

#ifndef _WIN32
struct sigaction oldAction;
#endif
bool printProcessIdDone = false;
string identForOpenlog;

//
// Should be called with staticMutex locked
//
size_t instanceCount()
{
    if(instanceList == 0)
    {
        return 0;
    }
    else
    {
        return instanceList->size();
    }
}

class Init
{
public:

    Init()
    {
        staticMutex = new IceUtil::Mutex;

        //
        // Although probably not necessary here, we consistently lock
        // staticMutex before accessing instanceList
        //
        IceUtilInternal::MutexPtrLock<IceUtil::Mutex> sync(staticMutex);
        instanceList = new std::list<IceInternal::Instance*>;
    }

    ~Init()
    {
        {
            IceUtilInternal::MutexPtrLock<IceUtil::Mutex> sync(staticMutex);
            int notDestroyedCount = 0;

            for(std::list<IceInternal::Instance*>::const_iterator p = instanceList->begin();
                p != instanceList->end(); ++p)
            {
                if(!(*p)->destroyed())
                {
                    notDestroyedCount++;
                }
            }

            if(notDestroyedCount > 0)
            {
                cerr << "!! " << IceUtil::Time::now().toDateTime() << " error: ";
                if(notDestroyedCount == 1)
                {
                    cerr << "communicator ";
                }
                else
                {
                    cerr << notDestroyedCount << " communicators ";
                }
                cerr << "not destroyed during global destruction.";
            }

            delete instanceList;
            instanceList = 0;
        }
        delete staticMutex;
        staticMutex = 0;
    }
};

Init init;

//
// Static initializer to register plugins.
//
IceInternal::RegisterPluginsInit initPlugins;

}

namespace IceInternal // Required because ObserverUpdaterI is a friend of Instance
{

class ObserverUpdaterI : public Ice::Instrumentation::ObserverUpdater
{
public:

    ObserverUpdaterI(const InstancePtr&);

    virtual void updateConnectionObservers();
    virtual void updateThreadObservers();

private:

    const InstancePtr _instance;
};


//
// Timer specialization which supports the thread observer
//
class Timer : public IceUtil::Timer
{
public:

    Timer(int priority) :
        IceUtil::Timer(priority),
        _hasObserver(0)
    {
    }

    Timer() :
        _hasObserver(0)
    {
    }

    void updateObserver(const Ice::Instrumentation::CommunicatorObserverPtr&);

private:

    virtual void runTimerTask(const IceUtil::TimerTaskPtr&);

    IceUtil::Mutex _mutex;
    //
    // TODO: Replace by std::atomic<bool> when it becomes widely
    // available.
    //
    IceUtilInternal::Atomic _hasObserver;
    ObserverHelperT<Ice::Instrumentation::ThreadObserver> _observer;
};

}

void
Timer::updateObserver(const Ice::Instrumentation::CommunicatorObserverPtr& obsv)
{
    IceUtil::Mutex::Lock sync(_mutex);
    assert(obsv);
    _observer.attach(obsv->getThreadObserver("Communicator",
                                            "Ice.Timer",
                                            Ice::Instrumentation::ThreadStateIdle,
                                            _observer.get()));
    _hasObserver.exchange(_observer.get() ? 1 : 0);
}

void
Timer::runTimerTask(const IceUtil::TimerTaskPtr& task)
{
    if(_hasObserver != 0)
    {
        Ice::Instrumentation::ThreadObserverPtr threadObserver;
        {
            IceUtil::Mutex::Lock sync(_mutex);
            threadObserver = _observer.get();
        }
        if(threadObserver)
        {
            threadObserver->stateChanged(Ice::Instrumentation::ThreadStateIdle,
                                         Ice::Instrumentation::ThreadStateInUseForOther);
        }
        try
        {
            task->runTimerTask();
        }
        catch(...)
        {
            if(threadObserver)
            {
                threadObserver->stateChanged(Ice::Instrumentation::ThreadStateInUseForOther,
                                             Ice::Instrumentation::ThreadStateIdle);
            }
        }
        if(threadObserver)
        {
            threadObserver->stateChanged(Ice::Instrumentation::ThreadStateInUseForOther,
                                         Ice::Instrumentation::ThreadStateIdle);
        }
    }
    else
    {
        task->runTimerTask();
    }
}

IceUtil::Shared* IceInternal::upCast(Instance* p) { return p; }

IceInternal::ObserverUpdaterI::ObserverUpdaterI(const InstancePtr& instance) : _instance(instance)
{
}

void
IceInternal::ObserverUpdaterI::updateConnectionObservers()
{
    _instance->updateConnectionObservers();
}

void
IceInternal::ObserverUpdaterI::updateThreadObservers()
{
    _instance->updateThreadObservers();
}

bool
IceInternal::Instance::destroyed() const
{
    Lock sync(*this);
    return _state == StateDestroyed;
}

TraceLevelsPtr
IceInternal::Instance::traceLevels() const
{
    // No mutex lock, immutable.
    assert(_traceLevels);
    return _traceLevels;
}

DefaultsAndOverridesPtr
IceInternal::Instance::defaultsAndOverrides() const
{
    // No mutex lock, immutable.
    assert(_defaultsAndOverrides);
    return _defaultsAndOverrides;
}

RouterManagerPtr
IceInternal::Instance::routerManager() const
{
    Lock sync(*this);

    if(_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    assert(_routerManager);
    return _routerManager;
}

LocatorManagerPtr
IceInternal::Instance::locatorManager() const
{
    Lock sync(*this);

    if(_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    assert(_locatorManager);
    return _locatorManager;
}

ReferenceFactoryPtr
IceInternal::Instance::referenceFactory() const
{
    Lock sync(*this);

    if(_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    assert(_referenceFactory);
    return _referenceFactory;
}

RequestHandlerFactoryPtr
IceInternal::Instance::requestHandlerFactory() const
{
    Lock sync(*this);

    if(_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    assert(_requestHandlerFactory);
    return _requestHandlerFactory;
}

ProxyFactoryPtr
IceInternal::Instance::proxyFactory() const
{
    Lock sync(*this);

    if(_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    assert(_proxyFactory);
    return _proxyFactory;
}

OutgoingConnectionFactoryPtr
IceInternal::Instance::outgoingConnectionFactory() const
{
    Lock sync(*this);

    if(_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    assert(_outgoingConnectionFactory);
    return _outgoingConnectionFactory;
}

ObjectFactoryManagerPtr
IceInternal::Instance::servantFactoryManager() const
{
    Lock sync(*this);

    if(_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    assert(_servantFactoryManager);
    return _servantFactoryManager;
}

ObjectAdapterFactoryPtr
IceInternal::Instance::objectAdapterFactory() const
{
    Lock sync(*this);

    if(_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    assert(_objectAdapterFactory);
    return _objectAdapterFactory;
}

ProtocolSupport
IceInternal::Instance::protocolSupport() const
{
    return _protocolSupport;
}

bool
IceInternal::Instance::preferIPv6() const
{
    return _preferIPv6;
}

NetworkProxyPtr
IceInternal::Instance::networkProxy() const
{
    return _networkProxy;
}

ThreadPoolPtr
IceInternal::Instance::clientThreadPool()
{
    Lock sync(*this);

    if(_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    assert(_clientThreadPool);
    return _clientThreadPool;
}

ThreadPoolPtr
IceInternal::Instance::serverThreadPool()
{
    Lock sync(*this);

    if(_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    if(!_serverThreadPool) // Lazy initialization.
    {
        if(_state == StateDestroyInProgress)
        {
            throw CommunicatorDestroyedException(__FILE__, __LINE__);
        }
        int timeout = _initData.properties->getPropertyAsInt("Ice.ServerIdleTime");
        _serverThreadPool = new ThreadPool(this, "Ice.ThreadPool.Server", timeout);
    }

    return _serverThreadPool;
}

EndpointHostResolverPtr
IceInternal::Instance::endpointHostResolver()
{
    Lock sync(*this);

    if(_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    assert(_endpointHostResolver);
    return _endpointHostResolver;
}

RetryQueuePtr
IceInternal::Instance::retryQueue()
{
    Lock sync(*this);

    if(_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    assert(_retryQueue);
    return _retryQueue;
}

IceUtil::TimerPtr
IceInternal::Instance::timer()
{
    Lock sync(*this);

    if(_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    assert(_timer);
    return _timer;
}

EndpointFactoryManagerPtr
IceInternal::Instance::endpointFactoryManager() const
{
    Lock sync(*this);

    if(_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    assert(_endpointFactoryManager);
    return _endpointFactoryManager;
}

DynamicLibraryListPtr
IceInternal::Instance::dynamicLibraryList() const
{
    Lock sync(*this);

    if(_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    assert(_dynamicLibraryList);
    return _dynamicLibraryList;
}

PluginManagerPtr
IceInternal::Instance::pluginManager() const
{
    Lock sync(*this);

    if(_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    assert(_pluginManager);
    return _pluginManager;
}

const ACMConfig&
IceInternal::Instance::clientACM() const
{
    // No mutex lock, immutable.
    return _clientACM;
}

const ACMConfig&
IceInternal::Instance::serverACM() const
{
    // No mutex lock, immutable.
    return _serverACM;
}

Identity
IceInternal::Instance::stringToIdentity(const string& s) const
{
    //
    // This method only accepts printable ascii. Since printable ascii is a subset
    // of all narrow string encodings, it is not necessary to convert the string
    // from the native string encoding. Any characters other than printable-ASCII
    // will cause an IllegalArgumentException. Note that it can contain Unicode
    // encoded in the escaped form which is the reason why we call fromUTF8 after
    // unespcaping the printable ASCII string.
    //

    Identity ident;

    //
    // Find unescaped separator; note that the string may contain an escaped
    // backslash before the separator.
    //
    string::size_type slash = string::npos, pos = 0;
    while((pos = s.find('/', pos)) != string::npos)
    {
        int escapes = 0;
        while(static_cast<int>(pos)- escapes > 0 && s[pos - escapes - 1] == '\\')
        {
            escapes++;
        }

        //
        // We ignore escaped escapes
        //
        if(escapes % 2 == 0)
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
                ex.str = "unescaped backslash in identity `" + s + "'";
                throw ex;
            }
        }
        pos++;
    }

    if(slash == string::npos)
    {
        try
        {
            ident.name = IceUtilInternal::unescapeString(s, 0, s.size());
        }
        catch(const IceUtil::IllegalArgumentException& e)
        {
            IdentityParseException ex(__FILE__, __LINE__);
            ex.str = "invalid identity name `" + s + "': " + e.reason();
            throw ex;
        }
    }
    else
    {
        try
        {
            ident.category = IceUtilInternal::unescapeString(s, 0, slash);
        }
        catch(const IceUtil::IllegalArgumentException& e)
        {
            IdentityParseException ex(__FILE__, __LINE__);
            ex.str = "invalid category in identity `" + s + "': " + e.reason();
            throw ex;
        }
        if(slash + 1 < s.size())
        {
            try
            {
                ident.name = IceUtilInternal::unescapeString(s, slash + 1, s.size());
            }
            catch(const IceUtil::IllegalArgumentException& e)
            {
                IdentityParseException ex(__FILE__, __LINE__);
                ex.str = "invalid name in identity `" + s + "': " + e.reason();
                throw ex;
            }
        }
    }

    ident.name = UTF8ToNative(ident.name, _stringConverter);
    ident.category = UTF8ToNative(ident.category, _stringConverter);

    return ident;
}

string
IceInternal::Instance::identityToString(const Identity& ident) const
{
    //
    // This method returns the stringified identity. The returned string only
    // contains printable ascii. It can contain UTF8 in the escaped form.
    //
    string name = nativeToUTF8(ident.name, _stringConverter);
    string category = nativeToUTF8(ident.category, _stringConverter);

    if(category.empty())
    {
        return IceUtilInternal::escapeString(name, "/");
    }
    else
    {
        return IceUtilInternal::escapeString(category, "/") + '/' + IceUtilInternal::escapeString(name, "/");
    }
}

Ice::ObjectPrx
IceInternal::Instance::createAdmin(const ObjectAdapterPtr& adminAdapter, const Identity& adminIdentity)
{
    ObjectAdapterPtr adapter = adminAdapter;
    bool createAdapter = !adminAdapter;

    Lock sync(*this);

    if(_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    if(adminIdentity.name.empty())
    {
        throw Ice::IllegalIdentityException(__FILE__, __LINE__, adminIdentity);
    }

    if(_adminAdapter)
    {
        throw InitializationException(__FILE__, __LINE__, "Admin already created");
    }

    if(!_adminEnabled)
    {
        throw InitializationException(__FILE__, __LINE__, "Admin is disabled");
    }

    if(createAdapter)
    {
        if(_initData.properties->getProperty("Ice.Admin.Endpoints") != "")
        {
            adapter = _objectAdapterFactory->createObjectAdapter("Ice.Admin", 0);
        }
        else
        {
            throw InitializationException(__FILE__, __LINE__, "Ice.Admin.Endpoints is not set");
        }
    }

    _adminIdentity = adminIdentity;
    _adminAdapter = adapter;
    addAllAdminFacets();
    sync.release();

    if(createAdapter)
    {
        try
        {
            adapter->activate();
        }
        catch(...)
        {
            //
            // We clean it up, even through this error is not recoverable
            // (can't call again createAdmin after fixing the problem since all the facets
            // in the adapter are lost)
            //
            adapter->destroy();
            sync.acquire();
            _adminAdapter = 0;
            throw;
        }
    }
    setServerProcessProxy(adapter, adminIdentity);
    return adapter->createProxy(adminIdentity);
}

Ice::ObjectPrx
IceInternal::Instance::getAdmin()
{
    Lock sync(*this);

    if(_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    if(_adminAdapter)
    {
        return _adminAdapter->createProxy(_adminIdentity);
    }
    else if(_adminEnabled)
    {
        ObjectAdapterPtr adapter;
        if(_initData.properties->getProperty("Ice.Admin.Endpoints") != "")
        {
            adapter = _objectAdapterFactory->createObjectAdapter("Ice.Admin", 0);
        }
        else
        {
            return 0;
        }

        Identity adminIdentity;
        adminIdentity.name = "admin";
        adminIdentity.category = _initData.properties->getProperty("Ice.Admin.InstanceName");
        if(adminIdentity.category.empty())
        {
            adminIdentity.category = IceUtil::generateUUID();
        }

        _adminIdentity = adminIdentity;
        _adminAdapter = adapter;
        addAllAdminFacets();
        sync.release();
        try
        {
            adapter->activate();
        }
        catch(...)
        {
            //
            // We clean it up, even through this error is not recoverable
            // (can't call again createAdmin after fixing the problem since all the facets
            // in the adapter are lost)
            //
            adapter->destroy();
            sync.acquire();
            _adminAdapter = 0;
            throw;
        }

        setServerProcessProxy(adapter, adminIdentity);
        return adapter->createProxy(adminIdentity);
    }
    else
    {
        return 0;
    }
}

void
IceInternal::Instance::addAllAdminFacets()
{
    // must be called with this locked

    //
    // Add all facets to OA
    //
    FacetMap filteredFacets;

    for(FacetMap::iterator p = _adminFacets.begin(); p != _adminFacets.end(); ++p)
    {
        if(_adminFacetFilter.empty() || _adminFacetFilter.find(p->first) != _adminFacetFilter.end())
        {
            _adminAdapter->addFacet(p->second, _adminIdentity, p->first);
        }
        else
        {
            filteredFacets[p->first] = p->second;
        }
    }
    _adminFacets.swap(filteredFacets);
}

void
IceInternal::Instance::setServerProcessProxy(const ObjectAdapterPtr& adminAdapter, const Identity& adminIdentity)
{
    ObjectPrx admin = adminAdapter->createProxy(adminIdentity);
    LocatorPrx locator = adminAdapter->getLocator();
    const string serverId = _initData.properties->getProperty("Ice.Admin.ServerId");
    if(locator && serverId != "")
    {
        ProcessPrx process = ProcessPrx::uncheckedCast(admin->ice_facet("Process"));
        try
        {
            //
            // Note that as soon as the process proxy is registered, the communicator might be
            // shutdown by a remote client and admin facets might start receiving calls.
            //
            locator->getRegistry()->setServerProcessProxy(serverId, process);
        }
        catch(const ServerNotFoundException&)
        {
            if(_traceLevels->location >= 1)
            {
                Trace out(_initData.logger, _traceLevels->locationCat);
                out << "couldn't register server `" + serverId + "' with the locator registry:\n";
                out << "the server is not known to the locator registry";
            }

            throw InitializationException(__FILE__, __LINE__, "Locator `" + _proxyFactory->proxyToString(locator) +
                                          "' knows nothing about server `" + serverId + "'");
        }
        catch(const LocalException& ex)
        {
            if(_traceLevels->location >= 1)
            {
                Trace out(_initData.logger, _traceLevels->locationCat);
                out << "couldn't register server `" + serverId + "' with the locator registry:\n" << ex;
            }
            throw;
        }

        if(_traceLevels->location >= 1)
        {
            Trace out(_initData.logger, _traceLevels->locationCat);
            out << "registered server `" + serverId + "' with the locator registry";
        }
    }
}

void
IceInternal::Instance::addAdminFacet(const Ice::ObjectPtr& servant, const string& facet)
{
    Lock sync(*this);

    if(_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    if(_adminAdapter == 0 || (!_adminFacetFilter.empty() && _adminFacetFilter.find(facet) == _adminFacetFilter.end()))
    {
        if(_adminFacets.insert(FacetMap::value_type(facet, servant)).second == false)
        {
            throw AlreadyRegisteredException(__FILE__, __LINE__, "facet", facet);
        }
    }
    else
    {
        _adminAdapter->addFacet(servant, _adminIdentity, facet);
    }
}

Ice::ObjectPtr
IceInternal::Instance::removeAdminFacet(const string& facet)
{
    Lock sync(*this);

    if(_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    ObjectPtr result;

    if(_adminAdapter == 0 || (!_adminFacetFilter.empty() && _adminFacetFilter.find(facet) == _adminFacetFilter.end()))
    {
        FacetMap::iterator p = _adminFacets.find(facet);
        if(p == _adminFacets.end())
        {
            throw NotRegisteredException(__FILE__, __LINE__, "facet", facet);
        }
        else
        {
            result = p->second;
            _adminFacets.erase(p);
        }
    }
    else
    {
        result = _adminAdapter->removeFacet(_adminIdentity, facet);
    }

    return result;
}

Ice::ObjectPtr
IceInternal::Instance::findAdminFacet(const string& facet)
{
    Lock sync(*this);

    if(_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    ObjectPtr result;

    //
    // If the _adminAdapter was not yet created, or this facet is filtered out, we check _adminFacets
    //
    if(!_adminAdapter || (!_adminFacetFilter.empty() && _adminFacetFilter.find(facet) == _adminFacetFilter.end()))
    {
        FacetMap::iterator p = _adminFacets.find(facet);
        if(p != _adminFacets.end())
        {
            result = p->second;
        }
    }
    else
    {
        // Otherwise, just check the _adminAdapter
        result = _adminAdapter->findFacet(_adminIdentity, facet);
    }

    return result;
}

FacetMap
IceInternal::Instance::findAllAdminFacets()
{
    Lock sync(*this);

    if(_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    if(!_adminAdapter)
    {
        return _adminFacets;
    }
    else
    {
        FacetMap result = _adminAdapter->findAllFacets(_adminIdentity);
        if(!_adminFacets.empty())
        {
            // Also returns filtered facets
            result.insert(_adminFacets.begin(), _adminFacets.end());
        }
        return result;
    }
}

void
IceInternal::Instance::setDefaultLocator(const Ice::LocatorPrx& defaultLocator)
{
    Lock sync(*this);

    if(_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    _referenceFactory = _referenceFactory->setDefaultLocator(defaultLocator);
}

void
IceInternal::Instance::setDefaultRouter(const Ice::RouterPrx& defaultRouter)
{
    Lock sync(*this);

    if(_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    _referenceFactory = _referenceFactory->setDefaultRouter(defaultRouter);
}

void
IceInternal::Instance::setLogger(const Ice::LoggerPtr& logger)
{
    //
    // No locking, as it can only be called during plug-in loading
    //
    _initData.logger = logger;
}

void
IceInternal::Instance::setThreadHook(const Ice::ThreadNotificationPtr& threadHook)
{
    //
    // No locking, as it can only be called during plug-in loading
    //
    _initData.threadHook = threadHook;
}

namespace
{

bool logStdErrConvert = true;

}

IceInternal::Instance::Instance(const CommunicatorPtr& communicator, const InitializationData& initData) :
    _state(StateActive),
    _initData(initData),
    _messageSizeMax(0),
    _batchAutoFlushSize(0),
    _collectObjects(false),
    _implicitContext(0),
    _stringConverter(IceUtil::getProcessStringConverter()),
    _wstringConverter(IceUtil::getProcessWstringConverter()),
    _adminEnabled(false)
{
    try
    {
        __setNoDelete(true);
        {
            IceUtilInternal::MutexPtrLock<IceUtil::Mutex> sync(staticMutex);
            instanceList->push_back(this);

            if(!_initData.properties)
            {
                _initData.properties = createProperties();
            }

            if(!oneOfDone)
            {
                //
                // StdOut and StdErr redirection
                //
                string stdOutFilename = _initData.properties->getProperty("Ice.StdOut");
                string stdErrFilename = _initData.properties->getProperty("Ice.StdErr");

                if(stdOutFilename != "")
                {
                    FILE* file = IceUtilInternal::freopen(stdOutFilename, "a", stdout);
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
                    FILE* file = IceUtilInternal::freopen(stdErrFilename, "a", stderr);
                    if(file == 0)
                    {
                        FileException ex(__FILE__, __LINE__);
                        ex.path = stdErrFilename;
                        ex.error = getSystemErrno();
                        throw ex;
                    }
                }

                if(_initData.properties->getPropertyAsInt("Ice.NullHandleAbort") > 0)
                {
                    IceUtilInternal::nullHandleAbort = true;
                }

#ifdef NDEBUG
                if(_initData.properties->getPropertyAsIntWithDefault("Ice.PrintStackTraces", 0) > 0)
#else
                if(_initData.properties->getPropertyAsIntWithDefault("Ice.PrintStackTraces", 1) > 0)
#endif
                {
                    IceUtilInternal::printStackTraces = true;
                }

#ifndef _WIN32
                string newUser = _initData.properties->getProperty("Ice.ChangeUser");
                if(!newUser.empty())
                {
                    errno = 0;
                    struct passwd* pw = getpwnam(newUser.c_str());
                    if(!pw)
                    {
                        if(errno)
                        {
                            SyscallException ex(__FILE__, __LINE__);
                            ex.error = getSystemErrno();
                            throw ex;
                        }
                        else
                        {
                            InitializationException ex(__FILE__, __LINE__, "Unknown user account `" + newUser + "'");
                            throw ex;
                        }
                    }

                    if(setgid(pw->pw_gid) == -1)
                    {
                        SyscallException ex(__FILE__, __LINE__);
                        ex.error = getSystemErrno();
                        throw ex;
                    }

                    if(initgroups(pw->pw_name, pw->pw_gid) == -1)
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
                oneOfDone = true;
            }

            if(instanceCount() == 1)
            {
#if defined(_WIN32) && !defined(ICE_OS_WINRT)
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
                sigaction(SIGPIPE, &action, &oldAction);
                if(_initData.properties->getPropertyAsInt("Ice.UseSyslog") > 0)
                {
                    identForOpenlog = _initData.properties->getProperty("Ice.ProgramName");
                    if(identForOpenlog.empty())
                    {
                        identForOpenlog = "<Unknown Ice Program>";
                    }
                    openlog(identForOpenlog.c_str(), LOG_PID, LOG_USER);
                }
#else
                logStdErrConvert =
                    _initData.properties->getPropertyAsIntWithDefault("Ice.LogStdErr.Convert", 1) > 0 &&
                    _initData.properties->getProperty("Ice.StdErr").empty();
#endif
            }
        }


        if(!_initData.logger)
        {
            string logfile = _initData.properties->getProperty("Ice.LogFile");
#ifndef _WIN32
            if(_initData.properties->getPropertyAsInt("Ice.UseSyslog") > 0)
            {
                if(!logfile.empty())
                {
                    throw InitializationException(__FILE__, __LINE__, "Both syslog and file logger cannot be enabled.");
                }

                _initData.logger =
                    new SysLoggerI(_initData.properties->getProperty("Ice.ProgramName"),
                                   _initData.properties->getPropertyWithDefault("Ice.SyslogFacility", "LOG_USER"));
            }
            else
#endif
            if(!logfile.empty())
            {
                _initData.logger =
                        new LoggerI(_initData.properties->getProperty("Ice.ProgramName"), logfile, true, 0,
                                    _initData.properties->getPropertyAsIntWithDefault("Ice.LogFile.SizeMax", 0));
            }
            else
            {
                _initData.logger = getProcessLogger();
                if(LoggerIPtr::dynamicCast(_initData.logger))
                {
                    _initData.logger = new LoggerI(_initData.properties->getProperty("Ice.ProgramName"), "",
                                                   logStdErrConvert, _stringConverter);
                }
            }
        }

        const_cast<TraceLevelsPtr&>(_traceLevels) = new TraceLevels(_initData.properties);

        const_cast<DefaultsAndOverridesPtr&>(_defaultsAndOverrides) =
            new DefaultsAndOverrides(_initData.properties, _initData.logger);

        const ACMConfig defaultClientACM(_initData.properties, _initData.logger, "Ice.ACM", ACMConfig(false));
        const ACMConfig defaultServerACM(_initData.properties, _initData.logger, "Ice.ACM", ACMConfig(true));

        const_cast<ACMConfig&>(_clientACM) = ACMConfig(_initData.properties,
                                                       _initData.logger,
                                                       "Ice.ACM.Client",
                                                       defaultClientACM);

        const_cast<ACMConfig&>(_serverACM) = ACMConfig(_initData.properties,
                                                       _initData.logger,
                                                       "Ice.ACM.Server",
                                                       defaultServerACM);

        {
            static const int defaultMessageSizeMax = 1024;
            Int num = _initData.properties->getPropertyAsIntWithDefault("Ice.MessageSizeMax", defaultMessageSizeMax);
            if(num < 1 || static_cast<size_t>(num) > static_cast<size_t>(0x7fffffff / 1024))
            {
                const_cast<size_t&>(_messageSizeMax) = static_cast<size_t>(0x7fffffff);
            }
            else
            {
                // Property is in kilobytes, _messageSizeMax in bytes.
                const_cast<size_t&>(_messageSizeMax) = static_cast<size_t>(num) * 1024;
            }
        }

        if(_initData.properties->getProperty("Ice.BatchAutoFlushSize").empty() &&
           !_initData.properties->getProperty("Ice.BatchAutoFlush").empty())
        {
            if(_initData.properties->getPropertyAsInt("Ice.BatchAutoFlush") > 0)
            {
                const_cast<size_t&>(_batchAutoFlushSize) = _messageSizeMax;
            }
        }
        else
        {
            Int num = _initData.properties->getPropertyAsIntWithDefault("Ice.BatchAutoFlushSize", 1024); // 1MB default
            if(num < 1)
            {
                const_cast<size_t&>(_batchAutoFlushSize) = num;
            }
            else if(static_cast<size_t>(num) > static_cast<size_t>(0x7fffffff / 1024))
            {
                const_cast<size_t&>(_batchAutoFlushSize) = static_cast<size_t>(0x7fffffff);
            }
            else
            {
                // Property is in kilobytes, convert in bytes.
                const_cast<size_t&>(_batchAutoFlushSize) = static_cast<size_t>(num) * 1024;
            }
        }

        const_cast<bool&>(_collectObjects) = _initData.properties->getPropertyAsInt("Ice.CollectObjects") > 0;

        //
        // Client ACM enabled by default. Server ACM disabled by default.
        //
#ifndef ICE_OS_WINRT
        const_cast<ImplicitContextIPtr&>(_implicitContext) =
            ImplicitContextI::create(_initData.properties->getProperty("Ice.ImplicitContext"));
#endif
        _routerManager = new RouterManager;

        _locatorManager = new LocatorManager(_initData.properties);

        _referenceFactory = new ReferenceFactory(this, communicator);

        _requestHandlerFactory = new RequestHandlerFactory(this);

        _proxyFactory = new ProxyFactory(this);

        const bool isIPv6Supported = IceInternal::isIPv6Supported();
        const bool ipv4 = _initData.properties->getPropertyAsIntWithDefault("Ice.IPv4", 1) > 0;
        const bool ipv6 = _initData.properties->getPropertyAsIntWithDefault("Ice.IPv6", isIPv6Supported ? 1 : 0) > 0;
        if(!ipv4 && !ipv6)
        {
            throw InitializationException(__FILE__, __LINE__, "Both IPV4 and IPv6 support cannot be disabled.");
        }
        else if(ipv4 && ipv6)
        {
            _protocolSupport = EnableBoth;
        }
        else if(ipv4)
        {
            _protocolSupport = EnableIPv4;
        }
        else
        {
            _protocolSupport = EnableIPv6;
        }
        _preferIPv6 = _initData.properties->getPropertyAsInt("Ice.PreferIPv6Address") > 0;

        _networkProxy = IceInternal::createNetworkProxy(_initData.properties, _protocolSupport);

        _endpointFactoryManager = new EndpointFactoryManager(this);

        _dynamicLibraryList = new DynamicLibraryList;

        _pluginManager = new PluginManagerI(communicator, _dynamicLibraryList);

        _outgoingConnectionFactory = new OutgoingConnectionFactory(communicator, this);

        _servantFactoryManager = new ObjectFactoryManager();

        _objectAdapterFactory = new ObjectAdapterFactory(this, communicator);

        _retryQueue = new RetryQueue(this);

        //
        // When _wstringConverter isn't set, use the default Unicode wstring converter
        //
        if(!_wstringConverter)
        {
            _wstringConverter = new IceUtil::UnicodeWstringConverter;
        }

        __setNoDelete(false);
    }
    catch(...)
    {
        {
            IceUtilInternal::MutexPtrLock<IceUtil::Mutex> sync(staticMutex);
            instanceList->remove(this);
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

    assert(!_servantFactoryManager);
    assert(!_objectAdapterFactory);
    assert(!_clientThreadPool);
    assert(!_serverThreadPool);
    assert(!_endpointHostResolver);
    assert(!_retryQueue);
    assert(!_timer);
    assert(!_routerManager);
    assert(!_locatorManager);
    assert(!_endpointFactoryManager);
    assert(!_dynamicLibraryList);
    assert(!_pluginManager);

    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> sync(staticMutex);
    if(instanceList != 0)
    {
        instanceList->remove(this);
    }
    if(instanceCount() == 0)
    {
#if defined(_WIN32) && !defined(ICE_OS_WINRT)
        WSACleanup();
#endif

#ifndef _WIN32
        sigaction(SIGPIPE, &oldAction, 0);

        if(!identForOpenlog.empty())
        {
            closelog();
            identForOpenlog.clear();
        }
#endif
    }
}

void
IceInternal::Instance::finishSetup(int& argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
    //
    // Load plug-ins.
    //
    assert(!_serverThreadPool);
    PluginManagerI* pluginManagerImpl = dynamic_cast<PluginManagerI*>(_pluginManager.get());
    assert(pluginManagerImpl);
    pluginManagerImpl->loadPlugins(argc, argv);

    //
    // Add WS and WSS endpoint factories if TCP/SSL factories are installed.
    //
    EndpointFactoryPtr tcpFactory = _endpointFactoryManager->get(TCPEndpointType);
    if(tcpFactory)
    {
        ProtocolInstancePtr instance = new ProtocolInstance(communicator, WSEndpointType, "ws", false);
        _endpointFactoryManager->add(new WSEndpointFactory(instance, tcpFactory->clone(instance)));
    }
    EndpointFactoryPtr sslFactory = _endpointFactoryManager->get(SSLEndpointType);
    if(sslFactory)
    {
        ProtocolInstancePtr instance = new ProtocolInstance(communicator, WSSEndpointType, "wss", true);
        _endpointFactoryManager->add(new WSEndpointFactory(instance, sslFactory->clone(instance)));
    }

    //
    // Reset _stringConverter and _wstringConverter, in case a plugin changed them
    //
    _stringConverter = IceUtil::getProcessStringConverter();

    IceUtil::WstringConverterPtr newWstringConverter = IceUtil::getProcessWstringConverter();
    if(newWstringConverter)
    {
        _wstringConverter = newWstringConverter;
    }
    else if(!dynamic_cast<IceUtil::UnicodeWstringConverter*>(_wstringConverter.get()))
    {
        _wstringConverter = new IceUtil::UnicodeWstringConverter;
    }

    //
    // Create Admin facets, if enabled.
    //
    // Note that any logger-dependent admin facet must be created after we load all plugins,
    // since one of these plugins can be a Logger plugin that sets a new logger during loading
    //

    if(_initData.properties->getProperty("Ice.Admin.Enabled") == "")
    {
        _adminEnabled = _initData.properties->getProperty("Ice.Admin.Endpoints") != "";
    }
    else
    {
        _adminEnabled = _initData.properties->getPropertyAsInt("Ice.Admin.Enabled") > 0;
    }

    StringSeq facetSeq = _initData.properties->getPropertyAsList("Ice.Admin.Facets");
    if(!facetSeq.empty())
    {
        _adminFacetFilter.insert(facetSeq.begin(), facetSeq.end());
    }

    if(_adminEnabled)
    {
        //
        // Process facet
        //
        const string processFacetName = "Process";
        if(_adminFacetFilter.empty() || _adminFacetFilter.find(processFacetName) != _adminFacetFilter.end())
        {
            _adminFacets.insert(make_pair(processFacetName, new ProcessI(communicator)));
        }

        //
        // Logger facet
        //
        const string loggerFacetName = "Logger";
        if(_adminFacetFilter.empty() || _adminFacetFilter.find(loggerFacetName) != _adminFacetFilter.end())
        {
            LoggerAdminLoggerPtr logger = createLoggerAdminLogger(_initData.properties, _initData.logger);
            setLogger(logger);
            _adminFacets.insert(make_pair(loggerFacetName, logger->getFacet()));
        }

        //
        // Properties facet
        //
        const string propertiesFacetName = "Properties";
        PropertiesAdminIPtr propsAdmin;
        if(_adminFacetFilter.empty() || _adminFacetFilter.find(propertiesFacetName) != _adminFacetFilter.end())
        {
            propsAdmin = new PropertiesAdminI(_initData.properties, _initData.logger);
            _adminFacets.insert(make_pair(propertiesFacetName, propsAdmin));
        }

        //
        // Metrics facet
        //
        const string metricsFacetName = "Metrics";
        if(_adminFacetFilter.empty() || _adminFacetFilter.find(metricsFacetName) != _adminFacetFilter.end())
        {
            CommunicatorObserverIPtr observer = new CommunicatorObserverI(_initData);
            _initData.observer = observer;
            _adminFacets.insert(make_pair(metricsFacetName, observer->getFacet()));

            //
            // Make sure the metrics admin facet receives property updates.
            //
            if(propsAdmin)
            {
                propsAdmin->addUpdateCallback(observer->getFacet());
            }
        }
    }

    //
    // Set observer updater
    //
    if(_initData.observer)
    {
        _initData.observer->setObserverUpdater(new ObserverUpdaterI(this));
    }

    //
    // Create threads.
    //
    try
    {
        bool hasPriority = _initData.properties->getProperty("Ice.ThreadPriority") != "";
        int priority = _initData.properties->getPropertyAsInt("Ice.ThreadPriority");
        if(hasPriority)
        {
            _timer = new Timer(priority);
        }
        else
        {
            _timer = new Timer;
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        Error out(_initData.logger);
        out << "cannot create thread for timer:\n" << ex;
        throw;
    }

    try
    {
        _endpointHostResolver = new EndpointHostResolver(this);
    }
    catch(const IceUtil::Exception& ex)
    {
        Error out(_initData.logger);
        out << "cannot create thread for endpoint host resolver:\n" << ex;
        throw;
    }

    _clientThreadPool = new ThreadPool(this, "Ice.ThreadPool.Client", 0);

    //
    // The default router/locator may have been set during the loading of plugins.
    // Therefore we make sure it is not already set before checking the property.
    //
    if(!_referenceFactory->getDefaultRouter())
    {
        RouterPrx router = RouterPrx::uncheckedCast(_proxyFactory->propertyToProxy("Ice.Default.Router"));
        if(router)
        {
            _referenceFactory = _referenceFactory->setDefaultRouter(router);
        }
    }

    if(!_referenceFactory->getDefaultLocator())
    {
        LocatorPrx locator = LocatorPrx::uncheckedCast(_proxyFactory->propertyToProxy("Ice.Default.Locator"));
        if(locator)
        {
            _referenceFactory = _referenceFactory->setDefaultLocator(locator);
        }
    }

    //
    // Show process id if requested (but only once).
    //
    bool printProcessId = false;
    if(!printProcessIdDone && _initData.properties->getPropertyAsInt("Ice.PrintProcessId") > 0)
    {
        //
        // Safe double-check locking (no dependent variable!)
        //
        IceUtilInternal::MutexPtrLock<IceUtil::Mutex> sync(staticMutex);
        printProcessId = !printProcessIdDone;

        //
        // We anticipate: we want to print it once, and we don't care when.
        //
        printProcessIdDone = true;
    }

    if(printProcessId)
    {
#ifdef _MSC_VER
        cout << GetCurrentProcessId() << endl;
#else
        cout << getpid() << endl;
#endif
    }

    //
    // Server thread pool initialization is lazy in serverThreadPool().
    //

    //
    // An application can set Ice.InitPlugins=0 if it wants to postpone
    // initialization until after it has interacted directly with the
    // plug-ins.
    //
    if(_initData.properties->getPropertyAsIntWithDefault("Ice.InitPlugins", 1) > 0)
    {
        pluginManagerImpl->initializePlugins();
    }

    //
    // This must be done last as this call creates the Ice.Admin object adapter
    // and eventually register a process proxy with the Ice locator (allowing
    // remote clients to invoke Admin facets as soon as it's registered).
    //
    // Note: getAdmin here can return 0 and do nothing in the event the
    // application set Ice.Admin.Enabled but did not set Ice.Admin.Enpoints
    // and one or more of the properties required to create the Admin object.
    //
    if(_adminEnabled && _initData.properties->getPropertyAsIntWithDefault("Ice.Admin.DelayCreation", 0) <= 0)
    {
        getAdmin();
    }
}

void
IceInternal::Instance::destroy()
{
    {
        Lock sync(*this);

        //
        // If destroy is in progress, wait for it to be done. This is
        // necessary in case destroy() is called concurrently by
        // multiple threads.
        //
        while(_state == StateDestroyInProgress)
        {
            wait();
        }

        if(_state == StateDestroyed)
        {
            return;
        }
        _state = StateDestroyInProgress;
    }

    //
    // Shutdown and destroy all the incoming and outgoing Ice
    // connections and wait for the connections to be finished.
    //
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

    if(_retryQueue)
    {
        _retryQueue->destroy(); // Must be called before destroying thread pools.
    }

    if(_initData.observer)
    {
        CommunicatorObserverIPtr observer = CommunicatorObserverIPtr::dynamicCast(_initData.observer);
        if(observer)
        {
            observer->destroy(); // Break cyclic reference counts. Don't clear _observer, it's immutable.
        }
        _initData.observer->setObserverUpdater(0); // Break cyclic reference count.
    }

    LoggerAdminLoggerPtr logger = LoggerAdminLoggerPtr::dynamicCast(_initData.logger);
    if(logger)
    {
        //
        // This only disables the remote logging; we don't set or reset _initData.logger
        //
        logger->destroy();
    }

    //
    // Now, destroy the thread pools. This must be done *only* after
    // all the connections are finished (the connections destruction
    // can require invoking callbacks with the thread pools).
    //
    if(_serverThreadPool)
    {
        _serverThreadPool->destroy();
    }
    if(_clientThreadPool)
    {
        _clientThreadPool->destroy();
    }
    if(_endpointHostResolver)
    {
        _endpointHostResolver->destroy();
    }
    if(_timer)
    {
        _timer->destroy();
    }

    //
    // Wait for all the threads to be finished.
    //
    if(_clientThreadPool)
    {
        _clientThreadPool->joinWithAllThreads();
    }
    if(_serverThreadPool)
    {
        _serverThreadPool->joinWithAllThreads();
    }
#ifndef ICE_OS_WINRT
    if(_endpointHostResolver)
    {
        _endpointHostResolver->getThreadControl().join();
    }
#endif

    if(_servantFactoryManager)
    {
        _servantFactoryManager->destroy();
    }

    if(_routerManager)
    {
        _routerManager->destroy();
    }

    if(_locatorManager)
    {
        _locatorManager->destroy();
    }

    if(_endpointFactoryManager)
    {
        _endpointFactoryManager->destroy();
    }

    if(_initData.properties->getPropertyAsInt("Ice.Warn.UnusedProperties") > 0)
    {
        set<string> unusedProperties = static_cast<PropertiesI*>(_initData.properties.get())->getUnusedProperties();
        if(unusedProperties.size() != 0)
        {
            Warning out(_initData.logger);
            out << "The following properties were set but never read:";
            for(set<string>::const_iterator p = unusedProperties.begin(); p != unusedProperties.end(); ++p)
            {
                out << "\n    " << *p;
            }
        }
    }

    //
    // Destroy last so that a Logger plugin can receive all log/traces before its destruction.
    //
    if(_pluginManager)
    {
        _pluginManager->destroy();
    }

    {
        Lock sync(*this);

        _objectAdapterFactory = 0;
        _outgoingConnectionFactory = 0;
        _retryQueue = 0;

        _serverThreadPool = 0;
        _clientThreadPool = 0;
        _endpointHostResolver = 0;
        _timer = 0;

        _servantFactoryManager = 0;
        _referenceFactory = 0;
        _requestHandlerFactory = 0;
        _proxyFactory = 0;
        _routerManager = 0;
        _locatorManager = 0;
        _endpointFactoryManager = 0;
        _pluginManager = 0;
        _dynamicLibraryList = 0;

        _adminAdapter = 0;
        _adminFacets.clear();

        _state = StateDestroyed;
        notifyAll();
    }
}

void
IceInternal::Instance::updateConnectionObservers()
{
    try
    {
        assert(_outgoingConnectionFactory);
        _outgoingConnectionFactory->updateConnectionObservers();
        assert(_objectAdapterFactory);
        _objectAdapterFactory->updateObservers(&ObjectAdapterI::updateConnectionObservers);
    }
    catch(const Ice::CommunicatorDestroyedException&)
    {
    }
}

void
IceInternal::Instance::updateThreadObservers()
{
    try
    {
        if(_clientThreadPool)
        {
            _clientThreadPool->updateObservers();
        }
        if(_serverThreadPool)
        {
            _serverThreadPool->updateObservers();
        }
        assert(_objectAdapterFactory);
        _objectAdapterFactory->updateObservers(&ObjectAdapterI::updateThreadObservers);
        if(_endpointHostResolver)
        {
            _endpointHostResolver->updateObserver();
        }
        if(_timer)
        {
            _timer->updateObserver(_initData.observer);
        }
    }
    catch(const Ice::CommunicatorDestroyedException&)
    {
    }
}


BufSizeWarnInfo
IceInternal::Instance::getBufSizeWarn(Short type)
{
    IceUtil::Mutex::Lock lock(_setBufSizeWarnMutex);

    return getBufSizeWarnInternal(type);
}

BufSizeWarnInfo
IceInternal::Instance::getBufSizeWarnInternal(Short type)
{
    BufSizeWarnInfo info;
    map<Short, BufSizeWarnInfo>::iterator p = _setBufSizeWarn.find(type);
    if(p == _setBufSizeWarn.end())
    {
        info.sndWarn = false;
        info.sndSize = -1;
        info.rcvWarn = false;
        info.rcvSize = -1;
        _setBufSizeWarn.insert(make_pair(type, info));
    }
    else
    {
        info = p->second;
    }
    return info;
}

void
IceInternal::Instance::setSndBufSizeWarn(Short type, int size)
{
    IceUtil::Mutex::Lock lock(_setBufSizeWarnMutex);

    BufSizeWarnInfo info = getBufSizeWarnInternal(type);
    info.sndWarn = true;
    info.sndSize = size;
    _setBufSizeWarn[type] =  info;
}

void
IceInternal::Instance::setRcvBufSizeWarn(Short type, int size)
{
    IceUtil::Mutex::Lock lock(_setBufSizeWarnMutex);

    BufSizeWarnInfo info = getBufSizeWarnInternal(type);
    info.rcvWarn = true;
    info.rcvSize = size;
    _setBufSizeWarn[type] =  info;
}

IceInternal::ProcessI::ProcessI(const CommunicatorPtr& communicator) :
    _communicator(communicator)
{
}

void
IceInternal::ProcessI::shutdown(const Current&)
{
    _communicator->shutdown();
}

void
IceInternal::ProcessI::writeMessage(const string& message, Int fd, const Current&)
{
    switch(fd)
    {
        case 1:
        {
            cout << message << endl;
            break;
        }
        case 2:
        {
            cerr << message << endl;
            break;
        }
    }
}
