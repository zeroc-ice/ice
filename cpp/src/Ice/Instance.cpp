// Copyright (c) ZeroC, Inc.

#include "Instance.h"
#include "AddDefaultPluginFactories.h"
#include "CheckIdentity.h"
#include "ConnectionFactory.h"
#include "ConsoleUtil.h"
#include "DefaultsAndOverrides.h"
#include "EndpointFactoryManager.h"
#include "FileUtil.h"
#include "IPEndpointI.h" // For EndpointHostResolver
#include "Ice/Communicator.h"
#include "Ice/DefaultSliceLoader.h"
#include "Ice/Initialize.h"
#include "Ice/LocalExceptions.h"
#include "Ice/Locator.h"
#include "Ice/LoggerUtil.h"
#include "Ice/NativePropertiesAdmin.h"
#include "Ice/ObserverHelper.h"
#include "Ice/Properties.h"
#include "Ice/Router.h"
#include "Ice/StringUtil.h"
#include "Ice/UUID.h"
#include "InstrumentationI.h"
#include "LocatorInfo.h"
#include "LoggerAdminI.h"
#include "LoggerI.h"
#include "NetworkProxy.h"
#include "ObjectAdapterFactory.h"
#include "PluginManagerI.h"
#include "ReferenceFactory.h"
#include "RetryQueue.h"
#include "RouterInfo.h"
#include "SSL/SSLEngine.h"
#include "ThreadPool.h"
#include "TimeUtil.h"
#include "TraceLevels.h"

#include "DisableWarnings.h"

#include <cstdio>
#include <list>
#include <mutex>

#if defined(_WIN32)
#    include "SSL/SchannelEngine.h"
#elif defined(__APPLE__)
#    include "SSL/SecureTransportEngine.h"
#else
#    include "SSL/OpenSSLEngine.h"
#endif

#ifdef __APPLE__
#    include "OSLogLoggerI.h"
#endif

#ifndef _WIN32

#    if !defined(__APPLE__) || TARGET_OS_IPHONE == 0
#        include "SysLoggerI.h"
#    endif

#    include "SystemdJournalI.h"

#    include <csignal>
#    include <pwd.h>
#    include <sys/types.h>
#    include <syslog.h>
#endif

#if defined(__linux__) || defined(__GLIBC__)
#    include <grp.h> // for initgroups
#endif

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace
{
    mutex staticMutex;
    bool oneOffDone = false;
    std::list<IceInternal::Instance*>* instanceList = nullptr;

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
        if (instanceList == nullptr)
        {
            return 0;
        }
        else
        {
            return instanceList->size();
        }
    }

    void checkPrintStackTraces(const InitializationData& initData) noexcept
    {
#ifdef NDEBUG
        // Release build
        if (initData.properties->getIcePropertyAsInt("Ice.PrintStackTraces") > 0)
#else
        // Debug build
        if (initData.properties->getPropertyAsIntWithDefault("Ice.PrintStackTraces", 1) > 0)
#endif
        {
            try
            {
                LocalException::ice_enableStackTraceCollection();
            }
            catch (const std::exception& ex)
            {
                Warning out(initData.logger);
                out << "Cannot enable stack trace collection:\n" << ex;
                out << "\nYou can turn off this warning by setting Ice.PrintStackTraces=0";
            }
        }
    }

    class Init
    {
    public:
        Init() noexcept
        {
            // Although probably not necessary here, we consistently lock
            // staticMutex before accessing instanceList.
            lock_guard lock(staticMutex);
            instanceList = new std::list<IceInternal::Instance*>;
        }

        ~Init()
        {
            {
                lock_guard lock(staticMutex);
                int notDestroyedCount = 0;

                for (const auto& p : *instanceList)
                {
                    if (!p->destroyed())
                    {
                        notDestroyedCount++;
                    }
                }

                if (notDestroyedCount > 0)
                {
                    consoleErr << "!! " << timePointToDateTimeString(chrono::system_clock::now()) << " error: ";
                    if (notDestroyedCount == 1)
                    {
                        consoleErr << "communicator ";
                    }
                    else
                    {
                        consoleErr << notDestroyedCount << " communicators ";
                    }
                    consoleErr << "not destroyed during global destruction.";
                }

                delete instanceList;
                instanceList = nullptr;
            }
        }
    };

    Init init;
}

namespace IceInternal // Required because ObserverUpdaterI is a friend of Instance
{
    class ObserverUpdaterI : public Ice::Instrumentation::ObserverUpdater
    {
    public:
        ObserverUpdaterI(InstancePtr);

        void updateConnectionObservers() override;
        void updateThreadObservers() override;

    private:
        const InstancePtr _instance;
    };

    //
    // Timer specialization which supports the thread observer
    //
    class ThreadObserverTimer final : public IceInternal::Timer
    {
    public:
        ThreadObserverTimer() : _hasObserver(false) {}

        void updateObserver(const Ice::Instrumentation::CommunicatorObserverPtr&);

    private:
        void runTimerTask(const TimerTaskPtr&) final;

        std::mutex _mutex;
        std::atomic<bool> _hasObserver;
        ObserverHelperT<Ice::Instrumentation::ThreadObserver> _observer;
    };
}

void
ThreadObserverTimer::updateObserver(const Ice::Instrumentation::CommunicatorObserverPtr& obsv)
{
    lock_guard lock(_mutex);
    assert(obsv);
    _observer.attach(obsv->getThreadObserver(
        "Communicator",
        "Ice.Timer",
        Instrumentation::ThreadState::ThreadStateIdle,
        _observer.get()));
    _hasObserver.exchange(_observer);
}

void
ThreadObserverTimer::runTimerTask(const TimerTaskPtr& task)
{
    if (_hasObserver)
    {
        Ice::Instrumentation::ThreadObserverPtr threadObserver;
        {
            lock_guard lock(_mutex);
            threadObserver = _observer.get();
        }
        if (threadObserver)
        {
            threadObserver->stateChanged(
                Instrumentation::ThreadState::ThreadStateIdle,
                Instrumentation::ThreadState::ThreadStateInUseForOther);
        }
        try
        {
            task->runTimerTask();
        }
        catch (...)
        {
            if (threadObserver)
            {
                threadObserver->stateChanged(
                    Instrumentation::ThreadState::ThreadStateInUseForOther,
                    Instrumentation::ThreadState::ThreadStateIdle);
            }
            throw;
        }

        if (threadObserver)
        {
            threadObserver->stateChanged(
                Instrumentation::ThreadState::ThreadStateInUseForOther,
                Instrumentation::ThreadState::ThreadStateIdle);
        }
    }
    else
    {
        task->runTimerTask();
    }
}

IceInternal::ObserverUpdaterI::ObserverUpdaterI(InstancePtr instance) : _instance(std::move(instance)) {}

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
    lock_guard lock(_mutex);
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
    lock_guard lock(_mutex);

    if (_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    assert(_routerManager);
    return _routerManager;
}

LocatorManagerPtr
IceInternal::Instance::locatorManager() const
{
    lock_guard lock(_mutex);

    if (_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    assert(_locatorManager);
    return _locatorManager;
}

ReferenceFactoryPtr
IceInternal::Instance::referenceFactory() const
{
    lock_guard lock(_mutex);

    if (_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    assert(_referenceFactory);
    return _referenceFactory;
}

OutgoingConnectionFactoryPtr
IceInternal::Instance::outgoingConnectionFactory() const
{
    lock_guard lock(_mutex);

    if (_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    assert(_outgoingConnectionFactory);
    return _outgoingConnectionFactory;
}

ObjectAdapterFactoryPtr
IceInternal::Instance::objectAdapterFactory() const
{
    lock_guard lock(_mutex);

    if (_state == StateDestroyed)
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
    lock_guard lock(_mutex);

    if (_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    assert(_clientThreadPool);
    return _clientThreadPool;
}

ThreadPoolPtr
IceInternal::Instance::serverThreadPool()
{
    lock_guard lock(_mutex);

    if (_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    if (!_serverThreadPool) // Lazy initialization.
    {
        if (_state == StateDestroyInProgress)
        {
            throw CommunicatorDestroyedException(__FILE__, __LINE__);
        }
        int timeout = _initData.properties->getIcePropertyAsInt("Ice.ServerIdleTime");
        _serverThreadPool = ThreadPool::create(shared_from_this(), "Ice.ThreadPool.Server", timeout);
    }

    return _serverThreadPool;
}

EndpointHostResolverPtr
IceInternal::Instance::endpointHostResolver()
{
    lock_guard lock(_mutex);

    if (_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    assert(_endpointHostResolver);
    return _endpointHostResolver;
}

RetryQueuePtr
IceInternal::Instance::retryQueue()
{
    lock_guard lock(_mutex);

    if (_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    assert(_retryQueue);
    return _retryQueue;
}

IceInternal::TimerPtr
IceInternal::Instance::timer()
{
    lock_guard lock(_mutex);

    if (_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    assert(_timer);
    return _timer;
}

EndpointFactoryManagerPtr
IceInternal::Instance::endpointFactoryManager() const
{
    lock_guard lock(_mutex);

    if (_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    assert(_endpointFactoryManager);
    return _endpointFactoryManager;
}

PluginManagerPtr
IceInternal::Instance::pluginManager() const
{
    lock_guard lock(_mutex);

    if (_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    assert(_pluginManager);
    return _pluginManager;
}

ConnectionOptions
IceInternal::Instance::serverConnectionOptions(const string& adapterName) const
{
    assert(!adapterName.empty());
    string propertyPrefix = adapterName + ".Connection";

    const PropertiesPtr& properties = _initData.properties;

    ConnectionOptions connectionOptions;

    connectionOptions.connectTimeout = chrono::seconds(properties->getPropertyAsIntWithDefault(
        propertyPrefix + ".ConnectTimeout",
        static_cast<int>(_serverConnectionOptions.connectTimeout.count())));

    connectionOptions.closeTimeout = chrono::seconds(properties->getPropertyAsIntWithDefault(
        propertyPrefix + ".CloseTimeout",
        static_cast<int>(_serverConnectionOptions.closeTimeout.count())));

    connectionOptions.idleTimeout = chrono::seconds(properties->getPropertyAsIntWithDefault(
        propertyPrefix + ".IdleTimeout",
        static_cast<int>(_serverConnectionOptions.idleTimeout.count())));

    connectionOptions.enableIdleCheck = properties->getPropertyAsIntWithDefault(
                                            propertyPrefix + ".EnableIdleCheck",
                                            _serverConnectionOptions.enableIdleCheck ? 1 : 0) > 0;

    connectionOptions.inactivityTimeout = chrono::seconds(properties->getPropertyAsIntWithDefault(
        propertyPrefix + ".InactivityTimeout",
        static_cast<int>(_serverConnectionOptions.inactivityTimeout.count())));

    connectionOptions.maxDispatches = properties->getPropertyAsIntWithDefault(
        propertyPrefix + ".MaxDispatches",
        _serverConnectionOptions.maxDispatches);

    return connectionOptions;
}

Ice::ObjectPrx
IceInternal::Instance::createAdmin(const ObjectAdapterPtr& adminAdapter, const Identity& adminIdentity)
{
    ObjectAdapterPtr adapter = adminAdapter;
    bool createAdapter = !adminAdapter;

    unique_lock lock(_mutex);

    if (_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    checkIdentity(adminIdentity, __FILE__, __LINE__);

    if (_adminAdapter)
    {
        throw InitializationException(__FILE__, __LINE__, "Admin already created");
    }

    if (!_adminEnabled)
    {
        throw InitializationException(__FILE__, __LINE__, "Admin is disabled");
    }

    if (createAdapter)
    {
        if (_initData.properties->getIceProperty("Ice.Admin.Endpoints") != "")
        {
            adapter = _objectAdapterFactory->createObjectAdapter("Ice.Admin", nullopt, nullopt);
        }
        else
        {
            throw InitializationException(__FILE__, __LINE__, "Ice.Admin.Endpoints is not set");
        }
    }

    _adminIdentity = adminIdentity;
    _adminAdapter = adapter;
    addAllAdminFacets();
    lock.unlock();

    if (createAdapter)
    {
        try
        {
            adapter->activate();
        }
        catch (...)
        {
            //
            // We clean it up, even through this error is not recoverable
            // (can't call again createAdmin after fixing the problem since all the facets
            // in the adapter are lost)
            //
            adapter->destroy();
            lock.lock();
            _adminAdapter = nullptr;
            throw;
        }
    }
    setServerProcessProxy(adapter, adminIdentity);
    return adapter->createProxy(adminIdentity);
}

std::optional<ObjectPrx>
IceInternal::Instance::getAdmin()
{
    unique_lock lock(_mutex);

    if (_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    if (_adminAdapter)
    {
        return _adminAdapter->createProxy(_adminIdentity);
    }
    else if (_adminEnabled)
    {
        ObjectAdapterPtr adapter;
        if (_initData.properties->getIceProperty("Ice.Admin.Endpoints") != "")
        {
            adapter = _objectAdapterFactory->createObjectAdapter("Ice.Admin", nullopt, nullopt);
        }
        else
        {
            return nullopt;
        }

        Identity adminIdentity;
        adminIdentity.name = "admin";
        adminIdentity.category = _initData.properties->getIceProperty("Ice.Admin.InstanceName");
        if (adminIdentity.category.empty())
        {
            adminIdentity.category = Ice::generateUUID();
        }

        _adminIdentity = adminIdentity;
        _adminAdapter = adapter;
        addAllAdminFacets();
        lock.unlock();
        try
        {
            adapter->activate();
        }
        catch (...)
        {
            //
            // We clean it up, even through this error is not recoverable
            // (can't call again createAdmin after fixing the problem since all the facets
            // in the adapter are lost)
            //
            adapter->destroy();
            lock.lock();
            _adminAdapter = nullptr;
            throw;
        }

        setServerProcessProxy(adapter, adminIdentity);
        return adapter->createProxy(adminIdentity);
    }
    else
    {
        return nullopt;
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

    for (const auto& adminFacet : _adminFacets)
    {
        if (_adminFacetFilter.empty() || _adminFacetFilter.find(adminFacet.first) != _adminFacetFilter.end())
        {
            _adminAdapter->addFacet(adminFacet.second, _adminIdentity, adminFacet.first);
        }
        else
        {
            filteredFacets[adminFacet.first] = adminFacet.second;
        }
    }
    _adminFacets.swap(filteredFacets);
}

void
IceInternal::Instance::setServerProcessProxy(const ObjectAdapterPtr& adminAdapter, const Identity& adminIdentity)
{
    ObjectPrx admin = adminAdapter->createProxy(adminIdentity);
    optional<LocatorPrx> locator = adminAdapter->getLocator();
    const string serverId = _initData.properties->getIceProperty("Ice.Admin.ServerId");
    if (locator && serverId != "")
    {
        auto process = admin->ice_facet<ProcessPrx>("Process");
        try
        {
            //
            // Note that as soon as the process proxy is registered, the communicator might be
            // shutdown by a remote client and admin facets might start receiving calls.
            //
            locator->getRegistry()->setServerProcessProxy(serverId, process);
        }
        catch (const ServerNotFoundException&)
        {
            if (_traceLevels->location >= 1)
            {
                Trace out(_initData.logger, _traceLevels->locationCat);
                out << "couldn't register server '" + serverId + "' with the locator registry:\n";
                out << "the server is not known to the locator registry";
            }

            throw InitializationException(
                __FILE__,
                __LINE__,
                "Locator '" + locator->ice_toString() + "' knows nothing about server '" + serverId + "'");
        }
        catch (const LocalException& ex)
        {
            if (_traceLevels->location >= 1)
            {
                Trace out(_initData.logger, _traceLevels->locationCat);
                out << "couldn't register server '" + serverId + "' with the locator registry:\n" << ex;
            }
            throw;
        }

        if (_traceLevels->location >= 1)
        {
            Trace out(_initData.logger, _traceLevels->locationCat);
            out << "registered server '" + serverId + "' with the locator registry";
        }
    }
}

void
IceInternal::Instance::addAdminFacet(ObjectPtr servant, string facet)
{
    lock_guard lock(_mutex);

    if (_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    if (_adminAdapter == nullptr ||
        (!_adminFacetFilter.empty() && _adminFacetFilter.find(facet) == _adminFacetFilter.end()))
    {
        if (_adminFacets.insert(FacetMap::value_type(facet, servant)).second == false)
        {
            throw AlreadyRegisteredException(__FILE__, __LINE__, "facet", facet);
        }
    }
    else
    {
        _adminAdapter->addFacet(std::move(servant), _adminIdentity, std::move(facet));
    }
}

ObjectPtr
IceInternal::Instance::removeAdminFacet(string_view facet)
{
    lock_guard lock(_mutex);

    if (_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    ObjectPtr result;

    if (_adminAdapter == nullptr ||
        (!_adminFacetFilter.empty() && _adminFacetFilter.find(facet) == _adminFacetFilter.end()))
    {
        auto p = _adminFacets.find(facet);
        if (p == _adminFacets.end())
        {
            throw NotRegisteredException(__FILE__, __LINE__, "facet", string{facet});
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

ObjectPtr
IceInternal::Instance::findAdminFacet(string_view facet)
{
    lock_guard lock(_mutex);

    if (_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    ObjectPtr result;

    //
    // If the _adminAdapter was not yet created, or this facet is filtered out, we check _adminFacets
    //
    if (!_adminAdapter || (!_adminFacetFilter.empty() && _adminFacetFilter.find(facet) == _adminFacetFilter.end()))
    {
        auto p = _adminFacets.find(facet);
        if (p != _adminFacets.end())
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
    lock_guard lock(_mutex);

    if (_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    if (!_adminAdapter)
    {
        return _adminFacets;
    }
    else
    {
        FacetMap result = _adminAdapter->findAllFacets(_adminIdentity);
        if (!_adminFacets.empty())
        {
            // Also returns filtered facets
            result.insert(_adminFacets.begin(), _adminFacets.end());
        }
        return result;
    }
}

void
IceInternal::Instance::setDefaultLocator(const optional<LocatorPrx>& defaultLocator)
{
    lock_guard lock(_mutex);

    if (_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    _referenceFactory = _referenceFactory->setDefaultLocator(defaultLocator);
}

void
IceInternal::Instance::setDefaultRouter(const optional<RouterPrx>& defaultRouter)
{
    lock_guard lock(_mutex);

    if (_state == StateDestroyed)
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
IceInternal::Instance::setThreadHook(function<void()> threadStart, function<void()> threadStop)
{
    _initData.threadStart = std::move(threadStart);
    _initData.threadStop = std::move(threadStop);
}
namespace
{
    bool logStdErrConvert = true;
}

InstancePtr
IceInternal::Instance::create(const Ice::CommunicatorPtr& communicator, Ice::InitializationData initData)
{
    auto instance = shared_ptr<Instance>(new Instance(std::move(initData)));
    instance->initialize(communicator);
    return instance;
}

IceInternal::Instance::Instance(InitializationData initData)
    : _initData(std::move(initData)),
      _stringConverter(Ice::getProcessStringConverter()),
      _wstringConverter(Ice::getProcessWstringConverter()),
      _applicationSliceLoader(make_shared<CompositeSliceLoader>())
{
#if defined(ICE_USE_SCHANNEL)
    if (_initData.clientAuthenticationOptions && _initData.clientAuthenticationOptions->trustedRootCertificates)
    {
        CertDuplicateStore(_initData.clientAuthenticationOptions->trustedRootCertificates);
    }
#elif defined(ICE_USE_SECURE_TRANSPORT)
    if (_initData.clientAuthenticationOptions && _initData.clientAuthenticationOptions->trustedRootCertificates)
    {
        CFRetain(_initData.clientAuthenticationOptions->trustedRootCertificates);
    }
#endif
}

void
IceInternal::Instance::initialize(const Ice::CommunicatorPtr& communicator)
{
    try
    {
        {
            lock_guard lock(staticMutex);
            instanceList->push_back(this);

            if (!_initData.properties)
            {
                _initData.properties = createProperties();
            }

            if (!oneOffDone)
            {
                //
                // StdOut and StdErr redirection
                //
                string stdOutFilename = _initData.properties->getIceProperty("Ice.StdOut");
                string stdErrFilename = _initData.properties->getIceProperty("Ice.StdErr");

                if (stdOutFilename != "")
                {
                    FILE* file = IceInternal::freopen(stdOutFilename, "a", stdout);
                    if (file == nullptr)
                    {
                        throw FileException(__FILE__, __LINE__, stdOutFilename);
                    }
                }

                if (stdErrFilename != "")
                {
                    FILE* file = IceInternal::freopen(stdErrFilename, "a", stderr);
                    if (file == nullptr)
                    {
                        throw FileException(__FILE__, __LINE__, stdErrFilename);
                    }
                }

                oneOffDone = true;
            }

            if (instanceCount() == 1)
            {
#if defined(_WIN32)
                WORD version = MAKEWORD(1, 1);
                WSADATA data;
                if (WSAStartup(version, &data) != 0)
                {
                    throw SocketException(__FILE__, __LINE__, getSocketErrno());
                }
#endif

#ifndef _WIN32
                struct sigaction action;
                action.sa_handler = SIG_IGN;
                sigemptyset(&action.sa_mask);
                action.sa_flags = 0;
                sigaction(SIGPIPE, &action, &oldAction);
                if (_initData.properties->getIcePropertyAsInt("Ice.UseSyslog") > 0)
                {
                    identForOpenlog = _initData.properties->getIceProperty("Ice.ProgramName");
                    if (identForOpenlog.empty())
                    {
                        identForOpenlog = "<Unknown Ice Program>";
                    }
                    openlog(identForOpenlog.c_str(), LOG_PID, LOG_USER);
                }
#else
                logStdErrConvert = _initData.properties->getIcePropertyAsInt("Ice.LogStdErr.Convert") > 0 &&
                                   _initData.properties->getIceProperty("Ice.StdErr").empty();
#endif
            }
        }

        if (!_initData.logger)
        {
            string logfile = _initData.properties->getIceProperty("Ice.LogFile");
#if !defined(_WIN32) && (!defined(__APPLE__) || TARGET_OS_IPHONE == 0)
            if (_initData.properties->getIcePropertyAsInt("Ice.UseSyslog") > 0)
            {
                if (!logfile.empty())
                {
                    throw InitializationException(__FILE__, __LINE__, "Both syslog and file logger cannot be enabled.");
                }

                _initData.logger = make_shared<SysLoggerI>(
                    _initData.properties->getIceProperty("Ice.ProgramName"),
                    _initData.properties->getIceProperty("Ice.SyslogFacility"));
            }
            else
#endif

#ifdef __APPLE__
                if (!_initData.logger && _initData.properties->getIcePropertyAsInt("Ice.UseOSLog") > 0)
            {
                _initData.logger = make_shared<OSLogLoggerI>(_initData.properties->getIceProperty("Ice.ProgramName"));
            }
            else
#endif

#ifdef ICE_USE_SYSTEMD
                if (_initData.properties->getIcePropertyAsInt("Ice.UseSystemdJournal") > 0)
            {
                _initData.logger =
                    make_shared<SystemdJournalI>(_initData.properties->getIceProperty("Ice.ProgramName"));
            }
            else
#endif
                if (!logfile.empty())
            {
                int32_t sz = _initData.properties->getIcePropertyAsInt("Ice.LogFile.SizeMax");
                if (sz < 0)
                {
                    sz = 0;
                }
                _initData.logger = make_shared<LoggerI>(
                    _initData.properties->getIceProperty("Ice.ProgramName"),
                    logfile,
                    true,
                    static_cast<size_t>(sz));
            }
            else
            {
                _initData.logger = getProcessLogger();
                if (dynamic_pointer_cast<LoggerI>(_initData.logger))
                {
                    _initData.logger = make_shared<LoggerI>(
                        _initData.properties->getIceProperty("Ice.ProgramName"),
                        "",
                        logStdErrConvert);
                }
            }
        }
        assert(_initData.logger);

        // This affects the entire process.
        checkPrintStackTraces(_initData);

        const_cast<TraceLevelsPtr&>(_traceLevels) = make_shared<TraceLevels>(_initData.properties);

        const_cast<DefaultsAndOverridesPtr&>(_defaultsAndOverrides) =
            make_shared<DefaultsAndOverrides>(_initData.properties);

        const_cast<ConnectionOptions&>(_clientConnectionOptions) = readConnectionOptions("Ice.Connection.Client");
        const_cast<ConnectionOptions&>(_serverConnectionOptions) = readConnectionOptions("Ice.Connection.Server");

        {
            int32_t num = _initData.properties->getIcePropertyAsInt("Ice.MessageSizeMax");
            if (num < 1 || static_cast<size_t>(num) > static_cast<size_t>(0x7fffffff / 1024))
            {
                const_cast<size_t&>(_messageSizeMax) = static_cast<size_t>(0x7fffffff);
            }
            else
            {
                // Property is in kilobytes, _messageSizeMax in bytes.
                const_cast<size_t&>(_messageSizeMax) = static_cast<size_t>(num) * 1024;
            }
        }

        if (_initData.properties->getIceProperty("Ice.BatchAutoFlushSize").empty() &&
            !_initData.properties->getIceProperty("Ice.BatchAutoFlush").empty())
        {
            if (_initData.properties->getIcePropertyAsInt("Ice.BatchAutoFlush") > 0)
            {
                const_cast<size_t&>(_batchAutoFlushSize) = _messageSizeMax;
            }
        }
        else
        {
            int32_t num = _initData.properties->getIcePropertyAsInt("Ice.BatchAutoFlushSize"); // 1MB default
            if (num < 1)
            {
                const_cast<size_t&>(_batchAutoFlushSize) = static_cast<size_t>(num);
            }
            else if (static_cast<size_t>(num) > static_cast<size_t>(0x7fffffff / 1024))
            {
                const_cast<size_t&>(_batchAutoFlushSize) = static_cast<size_t>(0x7fffffff);
            }
            else
            {
                // Property is in kilobytes, convert in bytes.
                const_cast<size_t&>(_batchAutoFlushSize) = static_cast<size_t>(num) * 1024;
            }
        }

        {
            int32_t num = _initData.properties->getIcePropertyAsInt("Ice.ClassGraphDepthMax");
            if (num < 1 || static_cast<size_t>(num) > static_cast<size_t>(0x7fffffff))
            {
                const_cast<size_t&>(_classGraphDepthMax) = static_cast<size_t>(0x7fffffff);
            }
            else
            {
                const_cast<size_t&>(_classGraphDepthMax) = static_cast<size_t>(num);
            }
        }

        // Update _initData.sliceLoader.

        if (_initData.sliceLoader)
        {
            _applicationSliceLoader->add(_initData.sliceLoader);
        }

        auto compositeSliceLoader = make_shared<CompositeSliceLoader>();
        compositeSliceLoader->add(_applicationSliceLoader);
        compositeSliceLoader->add(DefaultSliceLoader::instance());
        _initData.sliceLoader = std::move(compositeSliceLoader);

        string toStringModeStr = _initData.properties->getIceProperty("Ice.ToStringMode");
        if (toStringModeStr == "ASCII")
        {
            const_cast<ToStringMode&>(_toStringMode) = ToStringMode::ASCII;
        }
        else if (toStringModeStr == "Compat")
        {
            const_cast<ToStringMode&>(_toStringMode) = ToStringMode::Compat;
        }
        else if (toStringModeStr != "Unicode")
        {
            throw InitializationException(
                __FILE__,
                __LINE__,
                "The value for Ice.ToStringMode must be Unicode, ASCII or Compat");
        }

        const_cast<bool&>(_acceptClassCycles) = _initData.properties->getIcePropertyAsInt("Ice.AcceptClassCycles") > 0;

        string implicitContextKind = _initData.properties->getIceProperty("Ice.ImplicitContext");
        if (implicitContextKind == "Shared")
        {
            _implicitContextKind = ImplicitContextKind::Shared;
            _sharedImplicitContext = std::make_shared<ImplicitContext>();
        }
        else if (implicitContextKind == "PerThread")
        {
            _implicitContextKind = ImplicitContextKind::PerThread;
        }
        else if (implicitContextKind == "None")
        {
            _implicitContextKind = ImplicitContextKind::None;
        }
        else
        {
            throw Ice::InitializationException(
                __FILE__,
                __LINE__,
                "'" + implicitContextKind + "' is not a valid value for Ice.ImplicitContext");
        }

        _routerManager = make_shared<RouterManager>();

        _locatorManager = make_shared<LocatorManager>(_initData.properties);

        _referenceFactory = make_shared<ReferenceFactory>(shared_from_this(), communicator);

        const bool isIPv6Supported = IceInternal::isIPv6Supported();
        const bool ipv4 = _initData.properties->getIcePropertyAsInt("Ice.IPv4") > 0;
        const bool ipv6 = isIPv6Supported ? (_initData.properties->getIcePropertyAsInt("Ice.IPv6") > 0) : false;
        if (!ipv4 && !ipv6)
        {
            throw InitializationException(__FILE__, __LINE__, "Both IPV4 and IPv6 support cannot be disabled.");
        }
        else if (ipv4 && ipv6)
        {
            _protocolSupport = EnableBoth;
        }
        else if (ipv4)
        {
            _protocolSupport = EnableIPv4;
        }
        else
        {
            _protocolSupport = EnableIPv6;
        }
        _preferIPv6 = _initData.properties->getIcePropertyAsInt("Ice.PreferIPv6Address") > 0;

        _networkProxy = IceInternal::createNetworkProxy(_initData.properties, _protocolSupport);

        _endpointFactoryManager = make_shared<EndpointFactoryManager>(shared_from_this());

        _pluginManager = make_shared<PluginManagerI>(communicator);

        _outgoingConnectionFactory = make_shared<OutgoingConnectionFactory>(communicator, shared_from_this());

        _objectAdapterFactory = make_shared<ObjectAdapterFactory>(shared_from_this(), communicator);

        _retryQueue = make_shared<RetryQueue>(shared_from_this());

        StringSeq retryValues = _initData.properties->getIcePropertyAsList("Ice.RetryIntervals");
        if (retryValues.size() == 0)
        {
            _retryIntervals.push_back(0);
        }
        else
        {
            for (const auto& retryValue : retryValues)
            {
                istringstream value(retryValue);

                int v;
                if (!(value >> v) || !value.eof())
                {
                    v = 0;
                }

                //
                // If -1 is the first value, no retry and wait intervals.
                //
                if (v == -1 && _retryIntervals.empty())
                {
                    break;
                }

                _retryIntervals.push_back(v > 0 ? v : 0);
            }
        }

#if defined(_WIN32)
        _sslEngine = make_shared<Ice::SSL::Schannel::SSLEngine>(shared_from_this());
#elif defined(__APPLE__)
        _sslEngine = make_shared<Ice::SSL::SecureTransport::SSLEngine>(shared_from_this());
#else
        _sslEngine = make_shared<Ice::SSL::OpenSSL::SSLEngine>(shared_from_this());
#endif
        _sslEngine->initialize();
    }
    catch (...)
    {
        {
            lock_guard lock(staticMutex);
            instanceList->remove(this);
        }
        destroy();
        throw;
    }
}

const Ice::ImplicitContextPtr&
IceInternal::Instance::getImplicitContext() const
{
    switch (_implicitContextKind)
    {
        case ImplicitContextKind::PerThread:
        {
            static thread_local std::map<const IceInternal::Instance*, ImplicitContextPtr> perThreadImplicitContextMap;
            auto it = perThreadImplicitContextMap.find(this);
            if (it == perThreadImplicitContextMap.end())
            {
                auto r = perThreadImplicitContextMap.emplace(this, std::make_shared<ImplicitContext>());
                return r.first->second;
            }
            else
            {
                return it->second;
            }
        }
        case ImplicitContextKind::Shared:
        {
            assert(_sharedImplicitContext);
            return _sharedImplicitContext;
        }
        default:
        {
            assert(_sharedImplicitContext == nullptr);
            assert(_implicitContextKind == ImplicitContextKind::None);
            return _sharedImplicitContext;
        }
    }
}

IceInternal::Instance::~Instance()
{
    assert(_state == StateDestroyed);
    assert(!_referenceFactory);
    assert(!_outgoingConnectionFactory);

    assert(!_objectAdapterFactory);
    assert(!_clientThreadPool);
    assert(!_serverThreadPool);
    assert(!_endpointHostResolver);
    assert(!_endpointHostResolverThread.joinable());
    assert(!_retryQueue);
    assert(!_timer);
    assert(!_routerManager);
    assert(!_locatorManager);
    assert(!_endpointFactoryManager);
    assert(!_pluginManager);

    // Join all the destroy threads.
    for (auto& thread : _destroyThreads)
    {
        if (thread.get_id() == std::this_thread::get_id()) // completed may release the last ref-count
        {
            // Detach to avoid a deadlock.
            thread.detach();
        }
        else
        {
            thread.join();
        }
    }

    lock_guard lock(staticMutex);
    if (instanceList != nullptr)
    {
        instanceList->remove(this);
    }
    if (instanceCount() == 0)
    {
#if defined(_WIN32)
        WSACleanup();
#endif

#ifndef _WIN32
        sigaction(SIGPIPE, &oldAction, nullptr);

        if (!identForOpenlog.empty())
        {
            closelog();
            identForOpenlog.clear();
        }
#endif
    }
}

void
IceInternal::Instance::finishSetup(int& argc, const char* argv[], const Ice::CommunicatorPtr& communicator)
{
    // Load plug-ins.
    assert(!_serverThreadPool);
    auto pluginManagerImpl = dynamic_pointer_cast<PluginManagerI>(_pluginManager);
    assert(pluginManagerImpl);

    addDefaultPluginFactories(_initData.pluginFactories);
    bool libraryLoaded = pluginManagerImpl->loadPlugins(argc, argv);

    // On Windows, if we loaded any plugin and stack trace collection is enabled, we need to call
    // ice_enableStackTraceCollection() again to refresh the module list. This refresh is fairly slow so we make it only
    // when necessary. Extra calls to ice_enableStackTraceCollection() are no-op on other platforms.
    if (libraryLoaded)
    {
        checkPrintStackTraces(_initData);
    }

    //
    // Initialize the endpoint factories once all the plugins are loaded. This gives
    // the opportunity for the endpoint factories to find underlying factories.
    //
    _endpointFactoryManager->initialize();

    //
    // Reset _stringConverter and _wstringConverter, in case a plugin changed them
    //
    _stringConverter = Ice::getProcessStringConverter();
    _wstringConverter = Ice::getProcessWstringConverter();

    //
    // Create Admin facets, if enabled.
    //
    // Note that any logger-dependent admin facet must be created after we load all plugins,
    // since one of these plugins can be a Logger plugin that sets a new logger during loading
    //

    if (_initData.properties->getIceProperty("Ice.Admin.Enabled") == "")
    {
        _adminEnabled = _initData.properties->getIceProperty("Ice.Admin.Endpoints") != "";
    }
    else
    {
        _adminEnabled = _initData.properties->getIcePropertyAsInt("Ice.Admin.Enabled") > 0;
    }

    StringSeq facetSeq = _initData.properties->getIcePropertyAsList("Ice.Admin.Facets");
    if (!facetSeq.empty())
    {
        _adminFacetFilter.insert(facetSeq.begin(), facetSeq.end());
    }

    if (_adminEnabled)
    {
        //
        // Process facet
        //
        const string processFacetName = "Process";
        if (_adminFacetFilter.empty() || _adminFacetFilter.find(processFacetName) != _adminFacetFilter.end())
        {
            _adminFacets.insert(make_pair(processFacetName, make_shared<ProcessI>(communicator)));
        }

        //
        // Logger facet
        //
        const string loggerFacetName = "Logger";
        if (_adminFacetFilter.empty() || _adminFacetFilter.find(loggerFacetName) != _adminFacetFilter.end())
        {
            LoggerAdminLoggerPtr logger = createLoggerAdminLogger(_initData.properties, _initData.logger);
            setLogger(logger);
            _adminFacets.insert(make_pair(loggerFacetName, logger->getFacet()));
        }

        //
        // Properties facet
        //
        const string propertiesFacetName = "Properties";
        NativePropertiesAdminPtr propsAdmin;
        if (_adminFacetFilter.empty() || _adminFacetFilter.find(propertiesFacetName) != _adminFacetFilter.end())
        {
            propsAdmin = make_shared<NativePropertiesAdmin>(_initData.properties, _initData.logger);
            _adminFacets.insert(make_pair(propertiesFacetName, propsAdmin));
        }

        //
        // Metrics facet
        //
        const string metricsFacetName = "Metrics";
        if (_adminFacetFilter.empty() || _adminFacetFilter.find(metricsFacetName) != _adminFacetFilter.end())
        {
            CommunicatorObserverIPtr observer = make_shared<CommunicatorObserverI>(_initData);
            _initData.observer = observer;
            _adminFacets.insert(make_pair(metricsFacetName, observer->getFacet()));

            //
            // Make sure the metrics admin facet receives property updates.
            //
            if (propsAdmin)
            {
                auto metricsAdmin = observer->getFacet();
                propsAdmin->addUpdateCallback([metricsAdmin](const PropertyDict& changes)
                                              { metricsAdmin->updated(changes); });
            }
        }
    }

    //
    // Set observer updater
    //
    if (_initData.observer)
    {
        _initData.observer->setObserverUpdater(make_shared<ObserverUpdaterI>(shared_from_this()));
    }

    //
    // Create threads.
    //
    try
    {
        _timer = make_shared<ThreadObserverTimer>();
    }
    catch (const Ice::Exception& ex)
    {
        Error out(_initData.logger);
        out << "cannot create thread for timer:\n" << ex;
        throw;
    }

    try
    {
        _endpointHostResolver = make_shared<EndpointHostResolver>(shared_from_this());
        _endpointHostResolverThread = std::thread([this] { _endpointHostResolver->run(); });
    }
    catch (const Ice::Exception& ex)
    {
        Error out(_initData.logger);
        out << "cannot create thread for endpoint host resolver:\n" << ex;
        throw;
    }

    _clientThreadPool = ThreadPool::create(shared_from_this(), "Ice.ThreadPool.Client", 0);

    //
    // The default router/locator may have been set during the loading of plugins.
    // Therefore we make sure it is not already set before checking the property.
    //
    if (!_referenceFactory->getDefaultRouter())
    {
        auto router = communicator->propertyToProxy<RouterPrx>("Ice.Default.Router");
        if (router)
        {
            _referenceFactory = _referenceFactory->setDefaultRouter(router);
        }
    }

    if (!_referenceFactory->getDefaultLocator())
    {
        auto locator = communicator->propertyToProxy<LocatorPrx>("Ice.Default.Locator");
        if (locator)
        {
            _referenceFactory = _referenceFactory->setDefaultLocator(locator);
        }
    }

    //
    // Show process id if requested (but only once).
    //
    bool printProcessId = false;
    if (!printProcessIdDone && _initData.properties->getIcePropertyAsInt("Ice.PrintProcessId") > 0)
    {
        //
        // Safe double-check locking (no dependent variable!)
        //
        lock_guard lock(staticMutex);
        printProcessId = !printProcessIdDone;

        //
        // We anticipate: we want to print it once, and we don't care when.
        //
        printProcessIdDone = true;
    }

    if (printProcessId)
    {
#ifdef _MSC_VER
        consoleOut << GetCurrentProcessId() << endl;
#else
        consoleOut << getpid() << endl;
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
    if (_initData.properties->getIcePropertyAsInt("Ice.InitPlugins") > 0)
    {
        pluginManagerImpl->initializePlugins();
    }

    //
    // This must be done last as this call creates the Ice.Admin object adapter
    // and eventually register a process proxy with the Ice locator (allowing
    // remote clients to invoke Admin facets as soon as it's registered).
    //
    // Note: getAdmin here can return 0 and do nothing in the event the
    // application set Ice.Admin.Enabled but did not set Ice.Admin.Endpoints
    // and one or more of the properties required to create the Admin object.
    //
    if (_adminEnabled && _initData.properties->getIcePropertyAsInt("Ice.Admin.DelayCreation") <= 0)
    {
        getAdmin();
    }
}

void
IceInternal::Instance::destroy() noexcept
{
    {
        unique_lock lock(_mutex);

        //
        // If destroy is in progress, wait for it to be done. This is
        // necessary in case destroy() is called concurrently by
        // multiple threads.
        //
        _conditionVariable.wait(lock, [this] { return _state != StateDestroyInProgress; });

        if (_state == StateDestroyed)
        {
            return;
        }
        _state = StateDestroyInProgress;
    }

    //
    // Shutdown and destroy all the incoming and outgoing Ice
    // connections and wait for the connections to be finished.
    //
    if (_objectAdapterFactory)
    {
        _objectAdapterFactory->shutdown();
    }

    if (_outgoingConnectionFactory)
    {
        _outgoingConnectionFactory->destroy();
    }

    if (_objectAdapterFactory)
    {
        _objectAdapterFactory->destroy();
    }

    if (_outgoingConnectionFactory)
    {
        _outgoingConnectionFactory->waitUntilFinished();
    }

    if (_retryQueue)
    {
        _retryQueue->destroy(); // Must be called before destroying thread pools.
    }

    if (_initData.observer)
    {
        CommunicatorObserverIPtr observer = dynamic_pointer_cast<CommunicatorObserverI>(_initData.observer);
        if (observer)
        {
            observer->destroy(); // Break cyclic reference counts. Don't clear _observer, it's immutable.
        }
        _initData.observer->setObserverUpdater(nullptr); // Break cyclic reference count.
    }

#if defined(ICE_USE_SCHANNEL)
    if (_initData.clientAuthenticationOptions && _initData.clientAuthenticationOptions->trustedRootCertificates)
    {
        CertCloseStore(_initData.clientAuthenticationOptions->trustedRootCertificates, 0);
        const_cast<Ice::InitializationData&>(_initData).clientAuthenticationOptions = nullopt;
    }
#elif defined(ICE_USE_SECURE_TRANSPORT)
    if (_initData.clientAuthenticationOptions && _initData.clientAuthenticationOptions->trustedRootCertificates)
    {
        CFRelease(_initData.clientAuthenticationOptions->trustedRootCertificates);
        const_cast<Ice::InitializationData&>(_initData).clientAuthenticationOptions = nullopt;
    }
#endif

    LoggerAdminLoggerPtr logger = dynamic_pointer_cast<LoggerAdminLogger>(_initData.logger);
    if (logger)
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
    if (_serverThreadPool)
    {
        _serverThreadPool->destroy();
    }
    if (_clientThreadPool)
    {
        _clientThreadPool->destroy();
    }
    if (_endpointHostResolver)
    {
        _endpointHostResolver->destroy();
    }
    if (_timer)
    {
        _timer->destroy();
    }

    //
    // Wait for all the threads to be finished.
    //
    if (_clientThreadPool)
    {
        _clientThreadPool->joinWithAllThreads();
    }
    if (_serverThreadPool)
    {
        _serverThreadPool->joinWithAllThreads();
    }
    if (_endpointHostResolverThread.joinable())
    {
        _endpointHostResolverThread.join();
    }

    if (_routerManager)
    {
        _routerManager->destroy();
    }

    if (_locatorManager)
    {
        _locatorManager->destroy();
    }

    if (_initData.properties->getIcePropertyAsInt("Ice.Warn.UnusedProperties") > 0)
    {
        set<string> unusedProperties = _initData.properties.get()->getUnusedProperties();
        if (unusedProperties.size() != 0)
        {
            Warning out(_initData.logger);
            out << "The following properties were set but never read:";
            for (const auto& unusedProp : unusedProperties)
            {
                out << "\n    " << unusedProp;
            }
        }
    }

    //
    // Destroy last so that a Logger plugin can receive all log/traces before its destruction.
    //
    if (_pluginManager)
    {
        _pluginManager->destroy();
    }

    {
        lock_guard lock(_mutex);

        _objectAdapterFactory = nullptr;
        _outgoingConnectionFactory = nullptr;
        _retryQueue = nullptr;

        _serverThreadPool = nullptr;
        _clientThreadPool = nullptr;
        _endpointHostResolver = nullptr;
        _timer = nullptr;

        _referenceFactory = nullptr;
        _routerManager = nullptr;
        _locatorManager = nullptr;
        _endpointFactoryManager = nullptr;
        _pluginManager = nullptr;

        _adminAdapter = nullptr;
        _adminFacets.clear();

        _sslEngine = nullptr;

        _state = StateDestroyed;
        _conditionVariable.notify_all();
    }
}

void
IceInternal::Instance::destroyAsync(function<void()> completed) noexcept
{
    assert(completed); // the caller (Communicator) makes sure completed is callable.

    bool executeCallback = false;
    {
        lock_guard lock(_mutex);
        if (_state == StateDestroyed)
        {
            executeCallback = true; // execute outside the lock
        }
        else
        {
            // Start a thread that calls destroy() and then executes the callback.
            // We join these threads in the destructor. It's important to capture this and not shared_from_this() here.
            _destroyThreads.emplace_back(
                [this, completed = std::move(completed)]()
                {
                    this->destroy();
                    completed();
                });
        }
    }

    if (executeCallback)
    {
        completed();
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
    catch (const Ice::CommunicatorDestroyedException&)
    {
    }
}

void
IceInternal::Instance::updateThreadObservers()
{
    try
    {
        if (_clientThreadPool)
        {
            _clientThreadPool->updateObservers();
        }
        if (_serverThreadPool)
        {
            _serverThreadPool->updateObservers();
        }
        assert(_objectAdapterFactory);
        _objectAdapterFactory->updateObservers(&ObjectAdapterI::updateThreadObservers);
        if (_endpointHostResolver)
        {
            _endpointHostResolver->updateObserver();
        }
        if (_timer)
        {
            _timer->updateObserver(_initData.observer);
        }
    }
    catch (const Ice::CommunicatorDestroyedException&)
    {
    }
}

BufSizeWarnInfo
IceInternal::Instance::getBufSizeWarn(int16_t type)
{
    lock_guard lock(_setBufSizeWarnMutex);

    return getBufSizeWarnInternal(type);
}

BufSizeWarnInfo
IceInternal::Instance::getBufSizeWarnInternal(int16_t type)
{
    BufSizeWarnInfo info;
    auto p = _setBufSizeWarn.find(type);
    if (p == _setBufSizeWarn.end())
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

ConnectionOptions
IceInternal::Instance::readConnectionOptions(const string& propertyPrefix) const
{
    const PropertiesPtr& properties = _initData.properties;
    ConnectionOptions connectionOptions;

    connectionOptions.connectTimeout =
        chrono::seconds(properties->getIcePropertyAsInt(propertyPrefix + ".ConnectTimeout"));

    connectionOptions.closeTimeout = chrono::seconds(properties->getIcePropertyAsInt(propertyPrefix + ".CloseTimeout"));
    connectionOptions.idleTimeout = chrono::seconds(properties->getIcePropertyAsInt(propertyPrefix + ".IdleTimeout"));
    connectionOptions.enableIdleCheck = properties->getIcePropertyAsInt(propertyPrefix + ".EnableIdleCheck") > 0;

    connectionOptions.inactivityTimeout =
        chrono::seconds(properties->getIcePropertyAsInt(propertyPrefix + ".InactivityTimeout"));

    connectionOptions.maxDispatches = properties->getIcePropertyAsInt(propertyPrefix + ".MaxDispatches");

    return connectionOptions;
}

void
IceInternal::Instance::setSndBufSizeWarn(int16_t type, int size)
{
    lock_guard lock(_setBufSizeWarnMutex);

    BufSizeWarnInfo info = getBufSizeWarnInternal(type);
    info.sndWarn = true;
    info.sndSize = size;
    _setBufSizeWarn[type] = info;
}

void
IceInternal::Instance::setRcvBufSizeWarn(int16_t type, int size)
{
    lock_guard lock(_setBufSizeWarnMutex);

    BufSizeWarnInfo info = getBufSizeWarnInternal(type);
    info.rcvWarn = true;
    info.rcvSize = size;
    _setBufSizeWarn[type] = info;
}

IceInternal::ProcessI::ProcessI(CommunicatorPtr communicator) : _communicator(std::move(communicator)) {}

void
IceInternal::ProcessI::shutdown(const Current&)
{
    _communicator->shutdown();
}

void
IceInternal::ProcessI::writeMessage(string message, int32_t fd, const Current&)
{
    switch (fd)
    {
        case 1:
        {
            consoleOut << message << endl;
            break;
        }
        case 2:
        {
            consoleErr << message << endl;
            break;
        }
    }
}
