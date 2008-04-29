// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
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
#include <Ice/SelectorThread.h>
#include <Ice/ConnectionFactory.h>
#include <Ice/ConnectionMonitor.h>
#include <Ice/ObjectFactoryManager.h>
#include <Ice/LocalException.h>
#include <Ice/ObjectAdapterFactory.h>
#include <Ice/Exception.h>
#include <Ice/PropertiesI.h>
#include <Ice/LoggerI.h>
#include <Ice/Network.h>
#include <Ice/EndpointFactoryManager.h>
#include <Ice/TcpEndpointI.h>
#include <Ice/UdpEndpointI.h>
#include <Ice/DynamicLibrary.h>
#include <Ice/PluginManagerI.h>
#include <Ice/Initialize.h>
#include <Ice/LoggerUtil.h>
#include <IceUtil/StringUtil.h>
#include <Ice/PropertiesI.h>
#include <IceUtil/UUID.h>
#include <Ice/Communicator.h>

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

extern bool ICE_DECLSPEC_IMPORT nullHandleAbort;

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

ProtocolSupport
IceInternal::Instance::protocolSupport() const
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    return _protocolSupport;
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
        int timeout = _initData.properties->getPropertyAsInt("Ice.ServerIdleTime");
        _serverThreadPool = new ThreadPool(this, "Ice.ThreadPool.Server", timeout);
    }

    return _serverThreadPool;
}

SelectorThreadPtr
IceInternal::Instance::selectorThread()
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    
    if(!_selectorThread) // Lazy initialization.
    {
        _selectorThread = new SelectorThread(this);
    }

    return _selectorThread;
}

EndpointHostResolverPtr
IceInternal::Instance::endpointHostResolver()
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    
    if(!_endpointHostResolver) // Lazy initialization.
    {
        _endpointHostResolver = new EndpointHostResolver(this);
    }

    return _endpointHostResolver;
}

IceUtil::TimerPtr
IceInternal::Instance::timer()
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    if(!_timer) // Lazy initialization.
    {
        _timer = new IceUtil::Timer;
    }

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

    _defaultContext = new SharedContext(ctx); 
}

SharedContextPtr
IceInternal::Instance::getDefaultContext() const
{
    IceUtil::RecMutex::Lock sync(*this);
    
    if(_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    return _defaultContext;
}


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
        if(!IceUtilInternal::unescapeString(s, 0, s.size(), ident.name))
        {
            IdentityParseException ex(__FILE__, __LINE__);
            ex.str = s;
            throw ex;
        }
    }
    else
    {
        if(!IceUtilInternal::unescapeString(s, 0, slash, ident.category))
        {
            IdentityParseException ex(__FILE__, __LINE__);
            ex.str = s;
            throw ex;
        }
        if(slash + 1 < s.size())
        {
            if(!IceUtilInternal::unescapeString(s, slash + 1, s.size(), ident.name))
            {
                IdentityParseException ex(__FILE__, __LINE__);
                ex.str = s;
                throw ex;
            }
        }
    }

    if(_initData.stringConverter)
    {
        string tmpString;
        if(!ident.name.empty())
        {
            _initData.stringConverter->fromUTF8(reinterpret_cast<const Byte*>(ident.name.data()),
                                                reinterpret_cast<const Byte*>(ident.name.data() + ident.name.size()),
                                                tmpString);
            ident.name = tmpString;
        }

        if(!ident.category.empty())
        {
            _initData.stringConverter->fromUTF8(reinterpret_cast<const Byte*>(ident.category.data()),
                                                reinterpret_cast<const Byte*>(ident.category.data() +
                                                                             ident.category.size()),
                                                tmpString);
            ident.category = tmpString;
        }
    }

    return ident;
}

string
IceInternal::Instance::identityToString(const Identity& ident) const
{
    string name = ident.name;
    string category = ident.category;
    if(_initData.stringConverter)
    {
        UTF8BufferI buffer;
        Byte* last;
        if(!ident.name.empty())
        {
            last = _initData.stringConverter->toUTF8(ident.name.data(), ident.name.data() + ident.name.size(),
                                                           buffer);
            name = string(reinterpret_cast<const char*>(buffer.getBuffer()), last - buffer.getBuffer());
        }

        buffer.reset();
        if(!ident.category.empty())
        {
            last = _initData.stringConverter->toUTF8(ident.category.data(),
                                                     ident.category.data() + ident.category.size(), buffer);
            category = string(reinterpret_cast<const char*>(buffer.getBuffer()), last - buffer.getBuffer());
        }
    }

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
            _adminAdapter = _objectAdapterFactory->createObjectAdapter(adminOA, "", 0);

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
                sync.acquire();
                _adminAdapter = 0;
                adapter->destroy();
                throw;
            }
            
            if(defaultLocator != 0 && serverId != "")
            {    
                ProcessPrx process = ProcessPrx::uncheckedCast(
                    adapter->createProxy(_adminIdentity)->ice_facet("Process"));
                try
                {
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
            
            return adapter->createProxy(_adminIdentity);
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
    // No locking, as it can only be called during plugin loading
    //
    _initData.stringConverter = stringConverter;
}

void
IceInternal::Instance::setWstringConverter(const Ice::WstringConverterPtr& wstringConverter)
{
    //
    // No locking, as it can only be called during plugin loading
    //
    _initData.wstringConverter = wstringConverter;
}

void
IceInternal::Instance::setLogger(const Ice::LoggerPtr& logger)
{
    //
    // No locking, as it can only be called during plugin loading
    //
    _initData.logger = logger;
}

IceInternal::Instance::Instance(const CommunicatorPtr& communicator, const InitializationData& initData) :
    _state(StateActive),
    _initData(initData),
    _messageSizeMax(0),
    _clientACM(0),
    _serverACM(0),
    _defaultContext(new SharedContext),
    _implicitContext(0)
{
    try
    {
        __setNoDelete(true);

        IceUtil::StaticMutex::Lock sync(staticMutex);
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
		FILE* file = freopen(stdOutFilename.c_str(), "a", stdout);
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
                FILE* file = freopen(stdErrFilename.c_str(), "a", stderr);
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
        
        sync.release();
        

        if(!_initData.logger)
        {
#ifdef _WIN32
            //
            // DEPRECATED PROPERTY: Ice.UseEventLog is deprecated.
            //
            if(_initData.properties->getPropertyAsInt("Ice.UseEventLog") > 0)
            {
                _initData.logger = new EventLoggerI(_initData.properties->getProperty("Ice.ProgramName"));
            }
#else
            if(_initData.properties->getPropertyAsInt("Ice.UseSyslog") > 0)
            {
                _initData.logger = new SysLoggerI;
            }
#endif
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
        const_cast<ImplicitContextIPtr&>(_implicitContext) = 
            ImplicitContextI::create(_initData.properties->getProperty("Ice.ImplicitContext"));

        _routerManager = new RouterManager;

        _locatorManager = new LocatorManager;

        _referenceFactory = new ReferenceFactory(this, communicator);

        _proxyFactory = new ProxyFactory(this);

        bool ipv4 = _initData.properties->getPropertyAsIntWithDefault("Ice.IPv4", 1) > 0;
        bool ipv6 = _initData.properties->getPropertyAsIntWithDefault("Ice.IPv6", 0) > 0;
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

        _adminFacets.insert(FacetMap::value_type("Properties", new PropertiesAdminI(_initData.properties)));
        _adminFacets.insert(FacetMap::value_type("Process", new ProcessI(communicator)));

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
    assert(!_selectorThread);
    assert(!_endpointHostResolver);
    assert(!_timer);
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
        IceUtil::StaticMutex::Lock sync(staticMutex);
        printProcessId = !printProcessIdDone;
        
        //
        // We anticipate: we want to print it once, and we don't care when.
        //
        printProcessIdDone = true;
    }

    if(printProcessId)
    {
#ifdef _MSC_VER
        cout << _getpid() << endl;
#else
        cout << getpid() << endl;
#endif
    }
    
    if(_initData.properties->getPropertyAsIntWithDefault("Ice.Admin.DelayCreation", 0) <= 0)
    {
        getAdmin();
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
    interval = _initData.properties->getPropertyAsIntWithDefault("Ice.MonitorConnections", interval);
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
        _objectAdapterFactory->destroy();
    }

    if(_outgoingConnectionFactory)
    {
        _outgoingConnectionFactory->waitUntilFinished();
    }

    ThreadPoolPtr serverThreadPool;
    ThreadPoolPtr clientThreadPool;
    SelectorThreadPtr selectorThread;
    EndpointHostResolverPtr endpointHostResolver;

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

        if(_selectorThread)
        {
            _selectorThread->destroy();
            std::swap(selectorThread, _selectorThread);
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
    if(selectorThread)
    {
        selectorThread->joinWithThread();
    }
    if(endpointHostResolver)
    {
        endpointHostResolver->getThreadControl().join();
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
    return true;
}

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
