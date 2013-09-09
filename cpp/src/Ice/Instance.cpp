// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
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
#include <Ice/PropertiesI.h>
#include <Ice/PropertiesAdminI.h>
#include <Ice/LoggerI.h>
#include <Ice/Network.h>
#include <Ice/EndpointFactoryManager.h>
#include <Ice/RetryQueue.h>
#include <Ice/DynamicLibrary.h>
#include <Ice/PluginManagerI.h>
#include <Ice/Initialize.h>
#include <Ice/LoggerUtil.h>
#include <IceUtil/StringUtil.h>
#include <Ice/PropertiesI.h>
#include <Ice/Communicator.h>
#include <Ice/GC.h>
#include <Ice/MetricsAdminI.h>
#include <Ice/InstrumentationI.h>
 
#include <IceUtil/UUID.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/MutexPtrLock.h>

#include <stdio.h>

#ifndef _WIN32
#   include <Ice/SysLoggerI.h>

#   include <signal.h>
#   include <syslog.h>
#   include <pwd.h>
#   include <sys/types.h>
#endif

#if defined(__linux) || defined(__sun)
#   include <grp.h> // for initgroups
#endif

#include <Ice/UdpEndpointI.h>

#ifndef ICE_OS_WINRT
#   include <Ice/TcpEndpointI.h>
#else
#   include <IceSSL/EndpointInfo.h> // For IceSSL::EndpointType
#   include <Ice/winrt/StreamEndpointI.h>
#endif

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace IceUtilInternal
{

extern bool ICE_DECLSPEC_IMPORT nullHandleAbort;
extern bool ICE_DECLSPEC_IMPORT printStackTraces;

};

namespace IceInternal
{

extern IceUtil::Handle<IceInternal::GC> theCollector;

}

namespace
{

IceUtil::Mutex* staticMutex = 0;
bool oneOffDone = false;
int instanceCount = 0;
#ifndef _WIN32
struct sigaction oldAction;
#endif
bool printProcessIdDone = false;
string identForOpenlog;

class Init
{
public:

    Init()
    {
        staticMutex = new IceUtil::Mutex;
    }

    ~Init()
    {
        delete staticMutex;
        staticMutex = 0;
    }
};

Init init;

}

namespace IceInternal
{

class ObserverUpdaterI : public Ice::Instrumentation::ObserverUpdater
{
public:

    ObserverUpdaterI(InstancePtr instance) : _instance(instance)
    {
    }

    void updateConnectionObservers()
    {
        _instance->updateConnectionObservers();
    }

    void updateThreadObservers()
    {
        _instance->updateThreadObservers();
    }

private:

    InstancePtr _instance;
};

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
    IceUtil::RecMutex::Lock sync(*this);

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
    IceUtil::RecMutex::Lock sync(*this);

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
    IceUtil::RecMutex::Lock sync(*this);

    if(_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    assert(_referenceFactory);
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

    assert(_proxyFactory);
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

    assert(_outgoingConnectionFactory);
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

    assert(_connectionMonitor);
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

    assert(_servantFactoryManager);
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
    IceUtil::RecMutex::Lock sync(*this);

    if(_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    assert(_clientThreadPool);
    return _clientThreadPool;
}

ThreadPoolPtr
IceInternal::Instance::serverThreadPool(bool create)
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    
    if(!_serverThreadPool && create) // Lazy initialization.
    {
        int timeout = _initData.properties->getPropertyAsInt("Ice.ServerIdleTime");
        _serverThreadPool = new ThreadPool(this, "Ice.ThreadPool.Server", timeout);
    }

    return _serverThreadPool;
}

EndpointHostResolverPtr
IceInternal::Instance::endpointHostResolver()
{
    IceUtil::RecMutex::Lock sync(*this);

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
    IceUtil::RecMutex::Lock sync(*this);

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
    IceUtil::RecMutex::Lock sync(*this);

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
    IceUtil::RecMutex::Lock sync(*this);

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
    IceUtil::RecMutex::Lock sync(*this);

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
    IceUtil::RecMutex::Lock sync(*this);

    if(_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    assert(_pluginManager);
    return _pluginManager;
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

    ident.name = Ice::UTF8ToNative(_initData.stringConverter, ident.name);
    ident.category = Ice::UTF8ToNative(_initData.stringConverter, ident.category);

    return ident;
}

string
IceInternal::Instance::identityToString(const Identity& ident) const
{
    //
    // This method returns the stringified identity. The returned string only
    // contains printable ascii. It can contain UTF8 in the escaped form.
    //
    string name = Ice::nativeToUTF8(_initData.stringConverter, ident.name);
    string category = Ice::nativeToUTF8(_initData.stringConverter, ident.category);

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
IceInternal::Instance::getAdmin()
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    const string adminOA = "Ice.Admin";

    if(_adminAdapter != 0)
    {
        return _adminAdapter->createProxy(_adminIdentity);
    }
    else if(_initData.properties->getProperty(adminOA + ".Endpoints") == "")
    {
        return 0;
    }
    else
    {
        string serverId = _initData.properties->getProperty("Ice.Admin.ServerId");
        string instanceName = _initData.properties->getProperty("Ice.Admin.InstanceName");

        Ice::LocatorPrx defaultLocator = _referenceFactory->getDefaultLocator();
        if((defaultLocator != 0 && serverId != "") || instanceName != "")
        {
            if(_adminIdentity.name == "")
            {
                _adminIdentity.name = "admin";
                if(instanceName == "")
                {
                    instanceName = IceUtil::generateUUID();
                }
                _adminIdentity.category = instanceName;

                //
                // Afterwards, _adminIdentity is read-only
                //
            }

            //
            // Create OA
            //
            _adminAdapter = _objectAdapterFactory->createObjectAdapter(adminOA, 0);

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

            ObjectAdapterPtr adapter = _adminAdapter;
            sync.release();

            //
            // Activate OA
            //
            try
            {
                adapter->activate();
            }
            catch(...)
            {
                //
                // We cleanup _adminAdapter, however this error is not recoverable
                // (can't call again getAdmin() after fixing the problem)
                // since all the facets (servants) in the adapter are lost
                //
                adapter->destroy();
                sync.acquire();
                _adminAdapter = 0;
                throw;
            }
            
            Ice::ObjectPrx admin = adapter->createProxy(_adminIdentity);
            if(defaultLocator != 0 && serverId != "")
            {    
                ProcessPrx process = ProcessPrx::uncheckedCast(admin->ice_facet("Process"));
                try
                {
                    //
                    // Note that as soon as the process proxy is registered, the communicator might be 
                    // shutdown by a remote client and admin facets might start receiving calls.
                    //
                    defaultLocator->getRegistry()->setServerProcessProxy(serverId, process);
                }
                catch(const ServerNotFoundException&)
                {
                    if(_traceLevels->location >= 1)
                    {
                        Trace out(_initData.logger, _traceLevels->locationCat);
                        out << "couldn't register server `" + serverId + "' with the locator registry:\n";
                        out << "the server is not known to the locator registry";
                    }

                    throw InitializationException(__FILE__, __LINE__, "Locator knows nothing about server '" + 
                                                                      serverId + "'");
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
            }
            
            if(_traceLevels->location >= 1)
            {
                Trace out(_initData.logger, _traceLevels->locationCat);
                out << "registered server `" + serverId + "' with the locator registry";
            }
            
            return admin;
        }
        else
        {
            return 0;
        }
    }
}

void
IceInternal::Instance::addAdminFacet(const Ice::ObjectPtr& servant, const string& facet)
{
    IceUtil::RecMutex::Lock sync(*this);

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
    IceUtil::RecMutex::Lock sync(*this);
 
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
    IceUtil::RecMutex::Lock sync(*this);

    if(_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    ObjectPtr result;

    if(_adminAdapter == 0 || (!_adminFacetFilter.empty() && _adminFacetFilter.find(facet) == _adminFacetFilter.end()))
    {
        FacetMap::iterator p = _adminFacets.find(facet);
        if(p != _adminFacets.end())
        {
            result = p->second;
        }
    }
    else
    {
        result = _adminAdapter->findFacet(_adminIdentity, facet);
    }

    return result;
} 

void
IceInternal::Instance::setDefaultLocator(const Ice::LocatorPrx& defaultLocator)
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    _referenceFactory = _referenceFactory->setDefaultLocator(defaultLocator);
}

void 
IceInternal::Instance::setDefaultRouter(const Ice::RouterPrx& defaultRouter)
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    _referenceFactory = _referenceFactory->setDefaultRouter(defaultRouter);
}

void
IceInternal::Instance::setStringConverter(const Ice::StringConverterPtr& stringConverter)
{
    //
    // No locking, as it can only be called during plug-in loading
    //
    _initData.stringConverter = stringConverter;
}

void
IceInternal::Instance::setWstringConverter(const Ice::WstringConverterPtr& wstringConverter)
{
    //
    // No locking, as it can only be called during plug-in loading
    //
    _initData.wstringConverter = wstringConverter;
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

IceInternal::Instance::Instance(const CommunicatorPtr& communicator, const InitializationData& initData) :
    _state(StateActive),
    _initData(initData),
    _messageSizeMax(0),
    _clientACM(0),
    _serverACM(0),
    _implicitContext(0)
{
    try
    {
        __setNoDelete(true);
        {
            IceUtilInternal::MutexPtrLock<IceUtil::Mutex> sync(staticMutex);
            instanceCount++;
    
            if(!_initData.properties)
            {
                _initData.properties = createProperties();
            }
    
            if(!oneOffDone)
            {
                //
                // StdOut and StdErr redirection
                //
                string stdOutFilename = _initData.properties->getProperty("Ice.StdOut");
                string stdErrFilename = _initData.properties->getProperty("Ice.StdErr");
                
                if(stdOutFilename != "")
                {
#ifdef _LARGEFILE64_SOURCE
                    FILE* file = freopen64(stdOutFilename.c_str(), "a", stdout);
#else
#ifdef _WIN32
                    FILE* file = _wfreopen(IceUtil::stringToWstring(nativeToUTF8(_initData.stringConverter, 
                                                                                 stdOutFilename)).c_str(),
                                           L"a", stdout);
#else
                    FILE* file = freopen(stdOutFilename.c_str(), "a", stdout);
#endif
#endif
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
#ifdef _LARGEFILE64_SOURCE
                    FILE* file = freopen64(stdErrFilename.c_str(), "a", stderr);
#else
#ifdef _WIN32
                    FILE* file = _wfreopen(IceUtil::stringToWstring(nativeToUTF8(_initData.stringConverter,
                                                                                 stdErrFilename)).c_str(), 
                                           L"a", stderr);
#else
                    FILE* file = freopen(stdErrFilename.c_str(), "a", stderr);
#endif
#endif
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
                oneOffDone = true;
            }   
            
            if(instanceCount == 1)
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
                _initData.logger = new LoggerI(_initData.properties->getProperty("Ice.ProgramName"),
                                               nativeToUTF8(_initData.stringConverter, logfile));
            }
            else
            {
                _initData.logger = getProcessLogger();
            }
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

        //
        // Client ACM enabled by default. Server ACM disabled by default.
        //
        const_cast<Int&>(_clientACM) = _initData.properties->getPropertyAsIntWithDefault("Ice.ACM.Client", 60);
        const_cast<Int&>(_serverACM) = _initData.properties->getPropertyAsInt("Ice.ACM.Server");
#ifndef ICE_OS_WINRT
        const_cast<ImplicitContextIPtr&>(_implicitContext) = 
            ImplicitContextI::create(_initData.properties->getProperty("Ice.ImplicitContext"));
#endif
        _routerManager = new RouterManager;

        _locatorManager = new LocatorManager(_initData.properties);

        _referenceFactory = new ReferenceFactory(this, communicator);

        _proxyFactory = new ProxyFactory(this);

        string proxyHost = _initData.properties->getProperty("Ice.SOCKSProxyHost");
        int defaultIPv6 = 1; // IPv6 enabled by default.
        if(!proxyHost.empty())
        {
#ifdef ICE_OS_WINRT
            throw InitializationException(__FILE__, __LINE__, "SOCKS proxy not supported in WinRT");
#else
            int proxyPort = _initData.properties->getPropertyAsIntWithDefault("Ice.SOCKSProxyPort", 1080);
            _networkProxy = new SOCKSNetworkProxy(proxyHost, proxyPort);
            defaultIPv6 = 0; // IPv6 is not supported with SOCKS
#endif
        }

        bool ipv4 = _initData.properties->getPropertyAsIntWithDefault("Ice.IPv4", 1) > 0;
        bool ipv6 = _initData.properties->getPropertyAsIntWithDefault("Ice.IPv6", defaultIPv6) > 0;
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

#ifndef ICE_OS_WINRT
        if(ipv6 && SOCKSNetworkProxyPtr::dynamicCast(_networkProxy))
        {
            throw InitializationException(__FILE__, __LINE__, "IPv6 is not supported with SOCKS4 proxies");
        }
#endif

        _endpointFactoryManager = new EndpointFactoryManager(this);
#ifndef ICE_OS_WINRT
        EndpointFactoryPtr tcpEndpointFactory = new TcpEndpointFactory(this);
        _endpointFactoryManager->add(tcpEndpointFactory);
#else
        EndpointFactoryPtr tcpStreamEndpointFactory = new StreamEndpointFactory(this, TCPEndpointType);
        _endpointFactoryManager->add(tcpStreamEndpointFactory);
        EndpointFactoryPtr sslStreamEndpointFactory = new StreamEndpointFactory(this, IceSSL::EndpointType);
        _endpointFactoryManager->add(sslStreamEndpointFactory);
#endif
        EndpointFactoryPtr udpEndpointFactory = new UdpEndpointFactory(this);
        _endpointFactoryManager->add(udpEndpointFactory);

        _dynamicLibraryList = new DynamicLibraryList;

        _pluginManager = new PluginManagerI(communicator, _dynamicLibraryList);

        _outgoingConnectionFactory = new OutgoingConnectionFactory(communicator, this);

        _servantFactoryManager = new ObjectFactoryManager();

        _objectAdapterFactory = new ObjectAdapterFactory(this, communicator);
        
        _retryQueue = new RetryQueue(this);

        if(_initData.wstringConverter == 0)
        {
            _initData.wstringConverter = new UnicodeWstringConverter();
        }

        //
        // Add Process and Properties facets
        //

        StringSeq facetSeq = _initData.properties->getPropertyAsList("Ice.Admin.Facets");
        
        if(!facetSeq.empty())
        {
            _adminFacetFilter.insert(facetSeq.begin(), facetSeq.end());
        }

        _adminFacets.insert(FacetMap::value_type("Process", new ProcessI(communicator)));

        PropertiesAdminIPtr props = new PropertiesAdminI("Properties", _initData.properties, _initData.logger);
        _adminFacets.insert(FacetMap::value_type("Properties",props));

        _metricsAdmin = new MetricsAdminI(_initData.properties, _initData.logger);
        _adminFacets.insert(FacetMap::value_type("Metrics", _metricsAdmin));

        //
        // Setup the communicator observer only if the user didn't already set an
        // Ice observer resolver and if the admininistrative endpoints are set.
        //
        if((_adminFacetFilter.empty() || _adminFacetFilter.find("Metrics") != _adminFacetFilter.end()) &&
           _initData.properties->getProperty("Ice.Admin.Endpoints") != "")
        {
            _observer = new CommunicatorObserverI(_metricsAdmin, _initData.observer);

            //
            // Make sure the admin plugin receives property updates.
            //
            props->addUpdateCallback(_metricsAdmin);
        }
        else
        {
            _observer = _initData.observer;
        }

        __setNoDelete(false);
    }
    catch(...)
    {
        {
            IceUtilInternal::MutexPtrLock<IceUtil::Mutex> sync(staticMutex);
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
    assert(!_endpointHostResolver);
    assert(!_retryQueue);
    assert(!_timer);
    assert(!_routerManager);
    assert(!_locatorManager);
    assert(!_endpointFactoryManager);
    assert(!_dynamicLibraryList);
    assert(!_pluginManager);

    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> sync(staticMutex);
    if(--instanceCount == 0)
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
IceInternal::Instance::finishSetup(int& argc, char* argv[])
{
    //
    // Load plug-ins.
    //
    assert(!_serverThreadPool);
    PluginManagerI* pluginManagerImpl = dynamic_cast<PluginManagerI*>(_pluginManager.get());
    assert(pluginManagerImpl);
    pluginManagerImpl->loadPlugins(argc, argv);

    //
    // Set observer updater
    //
    if(_observer)
    {
        theCollector->updateObserver(_observer);
        _observer->setObserverUpdater(new ObserverUpdaterI(this));
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
            _timer = new IceUtil::Timer(priority);
        }
        else
        {
            _timer = new IceUtil::Timer;
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
    // Get default router and locator proxies. Don't move this
    // initialization before the plug-in initialization!!! The proxies
    // might depend on endpoint factories to be installed by plug-ins.
    //
    RouterPrx router = RouterPrx::uncheckedCast(_proxyFactory->propertyToProxy("Ice.Default.Router"));
    if(router)
    {
        _referenceFactory = _referenceFactory->setDefaultRouter(router);
    }

    LocatorPrx locator = LocatorPrx::uncheckedCast(_proxyFactory->propertyToProxy("Ice.Default.Locator"));
    if(locator)
    {
        _referenceFactory = _referenceFactory->setDefaultLocator(locator);
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
    // Create the connection monitor and ensure the interval for
    // monitoring connections is appropriate for client & server
    // ACM.
    //
    _connectionMonitor = new ConnectionMonitor(this, _initData.properties->getPropertyAsInt("Ice.MonitorConnections"));
    _connectionMonitor->checkIntervalForACM(_clientACM);
    _connectionMonitor->checkIntervalForACM(_serverACM);

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
    // remote clients to invoke on Ice.Admin facets as soon as it's registered).
    //
    if(_initData.properties->getPropertyAsIntWithDefault("Ice.Admin.DelayCreation", 0) <= 0)
    {
        getAdmin();
    }
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
        _objectAdapterFactory->destroy();
    }

    if(_outgoingConnectionFactory)
    {
        _outgoingConnectionFactory->waitUntilFinished();
    }

    if(_retryQueue)
    {
        _retryQueue->destroy();
    }

    if(_observer && theCollector)
    {
        theCollector->clearObserver(_observer);
    }

    if(_metricsAdmin)
    {
        _metricsAdmin->destroy();
        _metricsAdmin = 0;

        // Break cyclic reference counts. Don't clear _observer, it's immutable.
        if(_observer)
        {
            CommunicatorObserverIPtr::dynamicCast(_observer)->destroy(); 
        }
    }

    ThreadPoolPtr serverThreadPool;
    ThreadPoolPtr clientThreadPool;
    EndpointHostResolverPtr endpointHostResolver;
    {
        IceUtil::RecMutex::Lock sync(*this);

        _objectAdapterFactory = 0;
        _outgoingConnectionFactory = 0;
        _retryQueue = 0;

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
        if(_endpointHostResolver)
        {
            _endpointHostResolver->destroy();
            std::swap(endpointHostResolver, _endpointHostResolver);
        }

        if(_timer)
        {
            _timer->destroy();
            _timer = 0;
        }

        if(_servantFactoryManager)
        {
            _servantFactoryManager->destroy();
            _servantFactoryManager = 0;
        }
        
        //_referenceFactory->destroy(); // No destroy function defined.
        _referenceFactory = 0;
        
        // _proxyFactory->destroy(); // No destroy function defined.
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

        _adminAdapter = 0;
        _adminFacets.clear();

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
#ifndef ICE_OS_WINRT
    if(endpointHostResolver)
    {
        endpointHostResolver->getThreadControl().join();
    }
#endif
    
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
    return true;
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
        assert(theCollector);
        theCollector->updateObserver(_observer);
    }
    catch(const Ice::CommunicatorDestroyedException&)
    {
    }
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
