// Copyright (c) ZeroC, Inc.

#include "ServiceManagerI.h"
#include "../Ice/ConsoleUtil.h"
#include "../Ice/DynamicLibrary.h"
#include "../Ice/Instance.h"
#include "../Ice/Options.h"
#include "Ice/Ice.h"
#include "Ice/Initialize.h"
#include "Ice/StringUtil.h"

using namespace Ice;
using namespace IceInternal;
using namespace IceBox;
using namespace std;

using ServiceFactory = IceBox::Service* (*)(const CommunicatorPtr&);

namespace
{
    struct StartServiceInfo
    {
        StartServiceInfo(const std::string& service, const std::string& value, const Ice::StringSeq& serverArgs)
        {
            name = service;

            //
            // Split the entire property value into arguments. An entry point containing spaces
            // must be enclosed in quotes.
            //
            try
            {
                args = IceInternal::Options::split(value);
            }
            catch (const IceInternal::BadOptException& ex)
            {
                throw FailureException(
                    __FILE__,
                    __LINE__,
                    "ServiceManager: invalid arguments for service '" + name + "':\n" + string{ex.what()});
            }

            assert(!args.empty());

            //
            // Shift the arguments.
            //
            entryPoint = args[0];
            args.erase(args.begin());

            for (const auto& serverArg : serverArgs)
            {
                if (serverArg.find("--" + name + ".") == 0)
                {
                    args.push_back(serverArg);
                }
            }
        }

        string name;
        string entryPoint;
        Ice::StringSeq args;
    };

    inline bool printStackTraces(const PropertiesPtr& properties)
    {
#ifdef NDEBUG
        // Release build
        return properties->getIcePropertyAsInt("Ice.PrintStackTraces") > 0;
#else
        // Debug build
        return properties->getPropertyAsIntWithDefault("Ice.PrintStackTraces", 1) > 0;
#endif
    }
}

IceBox::ServiceManagerI::ServiceManagerI(CommunicatorPtr communicator, int& argc, char* argv[])
    : _communicator(std::move(communicator))
{
    _logger = _communicator->getLogger();

    PropertiesPtr props = _communicator->getProperties();
    _traceServiceObserver = props->getIcePropertyAsInt("IceBox.Trace.ServiceObserver");

    if (props->getIceProperty("Ice.Admin.Enabled") == "")
    {
        _adminEnabled = props->getIceProperty("Ice.Admin.Endpoints") != "";
    }
    else
    {
        _adminEnabled = props->getIcePropertyAsInt("Ice.Admin.Enabled") > 0;
    }

    if (_adminEnabled)
    {
        StringSeq facetSeq = props->getIcePropertyAsList("Ice.Admin.Facets");
        if (!facetSeq.empty())
        {
            _adminFacetFilter.insert(facetSeq.begin(), facetSeq.end());
        }
    }

    for (int i = 1; i < argc; i++)
    {
        _argv.emplace_back(argv[i]);
    }
}

void
IceBox::ServiceManagerI::startService(string name, const Current&)
{
    ServiceInfo info;
    {
        lock_guard<mutex> lock(_mutex);

        //
        // Search would be more efficient if services were contained in
        // a map, but order is required for shutdown.
        //
        vector<ServiceInfo>::iterator p;
        for (p = _services.begin(); p != _services.end(); ++p)
        {
            if (p->name == name)
            {
                if (p->status != Stopped)
                {
                    throw AlreadyStartedException();
                }
                p->status = Starting;
                info = *p;
                break;
            }
        }
        if (p == _services.end())
        {
            throw NoSuchServiceException();
        }
        _pendingStatusChanges = true;
    }

    bool started = false;
    try
    {
        info.service->start(name, info.communicator == nullptr ? _sharedCommunicator : info.communicator, info.args);
        started = true;
    }
    catch (const Exception& ex)
    {
        Warning out(_logger);
        out << "ServiceManager: exception in start for service " << info.name << ":\n";
        out << ex;
    }
    catch (...)
    {
        Warning out(_logger);
        out << "ServiceManager: unknown exception in start for service " << info.name;
    }

    {
        lock_guard<mutex> lock(_mutex);
        for (auto& service : _services)
        {
            if (service.name == name)
            {
                if (started)
                {
                    service.status = Started;

                    vector<string> services;
                    services.push_back(name);
                    servicesStarted(services, _observers);
                }
                else
                {
                    service.status = Stopped;
                }
                break;
            }
        }
        _pendingStatusChanges = false;
        _conditionVariable.notify_all();
    }
}

void
IceBox::ServiceManagerI::stopService(string name, const Current&)
{
    ServiceInfo info;
    {
        lock_guard<mutex> lock(_mutex);

        //
        // Search would be more efficient if services were contained in
        // a map, but order is required for shutdown.
        //
        vector<ServiceInfo>::iterator p;
        for (p = _services.begin(); p != _services.end(); ++p)
        {
            if (p->name == name)
            {
                if (p->status != Started)
                {
                    throw AlreadyStoppedException();
                }
                p->status = Stopping;
                info = *p;
                break;
            }
        }
        if (p == _services.end())
        {
            throw NoSuchServiceException();
        }
        _pendingStatusChanges = true;
    }

    bool stopped = false;
    try
    {
        info.service->stop();
        stopped = true;
    }
    catch (const Exception& ex)
    {
        Warning out(_logger);
        out << "ServiceManager: exception while stopping service " << info.name << ":\n";
        out << ex;
    }
    catch (...)
    {
        Warning out(_logger);
        out << "ServiceManager: unknown exception while stopping service " << info.name;
    }

    {
        lock_guard<mutex> lock(_mutex);
        for (auto& service : _services)
        {
            if (service.name == name)
            {
                if (stopped)
                {
                    service.status = Stopped;

                    vector<string> services;
                    services.push_back(name);
                    servicesStopped(services, _observers);
                }
                else
                {
                    service.status = Started;
                }
                break;
            }
        }
        _pendingStatusChanges = false;
        _conditionVariable.notify_all();
    }
}

void
IceBox::ServiceManagerI::addObserver(optional<ServiceObserverPrx> observer, const Current&)
{
    // Null observers are ignored
    if (observer)
    {
        addObserver(*std::move(observer));
    }
}

void
IceBox::ServiceManagerI::addObserver(ServiceObserverPrx observer) // NOLINT(performance-unnecessary-value-param)
{
    // Duplicate registrations are ignored

    lock_guard<mutex> lock(_mutex);
    if (_observers.insert(observer).second)
    {
        if (_traceServiceObserver >= 1)
        {
            Trace out(_logger, "IceBox.ServiceObserver");
            out << "Added service observer " << observer;
        }

        vector<string> activeServices;
        for (const auto& info : _services)
        {
            if (info.status == Started)
            {
                activeServices.push_back(info.name);
            }
        }

        if (activeServices.size() > 0)
        {
            observer->servicesStartedAsync(activeServices, nullptr, makeObserverCompletedCallback(observer));
        }
    }
}

void
IceBox::ServiceManagerI::shutdown(const Current&)
{
    _communicator->shutdown();
}

bool
IceBox::ServiceManagerI::start()
{
    try
    {
        ServiceManagerPtr obj(shared_from_this());
        PropertiesPtr properties = _communicator->getProperties();

        //
        // Parse the property set with the prefix "IceBox.Service.". These
        // properties should have the following format:
        //
        // IceBox.Service.Foo=entry_point [args]
        //
        // We parse the service properties specified in IceBox.LoadOrder
        // first, then the ones from remaining services.
        //
        const string prefix = "IceBox.Service.";
        PropertyDict services = properties->getPropertiesForPrefix(prefix);
        if (services.empty())
        {
            throw FailureException(
                __FILE__,
                __LINE__,
                "ServiceManager: configuration must include at least one IceBox service");
        }

        StringSeq loadOrder = properties->getIcePropertyAsList("IceBox.LoadOrder");
        vector<StartServiceInfo> servicesInfo;
        for (const auto& q : loadOrder)
        {
            auto p = services.find(prefix + q);
            if (p == services.end())
            {
                throw FailureException(__FILE__, __LINE__, "ServiceManager: no service definition for '" + q + "'");
            }
            servicesInfo.emplace_back(q, p->second, _argv);
            services.erase(p);
        }
        for (const auto& service : services)
        {
            servicesInfo.emplace_back(service.first.substr(prefix.size()), service.second, _argv);
        }

        //
        // Check if some services are using the shared communicator in which
        // case we create the shared communicator now with a property set that
        // is the union of all the service properties (from services that use
        // the shared communicator).
        //
        PropertyDict sharedCommunicatorServices = properties->getPropertiesForPrefix("IceBox.UseSharedCommunicator.");
        if (!sharedCommunicatorServices.empty())
        {
            InitializationData initData;
            initData.properties = createServiceProperties("SharedCommunicator");

            for (auto& info : servicesInfo)
            {
                if (properties->getIcePropertyAsInt("IceBox.UseSharedCommunicator." + info.name) <= 0)
                {
                    continue;
                }

                //
                // Load the service properties using the shared communicator properties as
                // the default properties.
                //
                PropertiesPtr svcProperties = createProperties(info.args, initData.properties);

                //
                // Remove properties from the shared property set that a service explicitly clears.
                //
                PropertyDict allProps = initData.properties->getPropertiesForPrefix("");
                for (const auto& p : allProps)
                {
                    if (svcProperties->getProperty(p.first) == "")
                    {
                        initData.properties->setProperty(p.first, "");
                    }
                }

                //
                // Add the service properties to the shared communicator properties.
                //
                PropertyDict props = svcProperties->getPropertiesForPrefix("");
                for (const auto& r : props)
                {
                    initData.properties->setProperty(r.first, r.second);
                }

                //
                // Parse <service>.* command line options (the Ice command line options
                // were parsed by the call to createProperties above).
                //
                info.args = initData.properties->parseCommandLineOptions(info.name, info.args);
            }

            const string facetNamePrefix = "IceBox.SharedCommunicator.";
            bool addFacets = configureAdmin(initData.properties, facetNamePrefix);

            _sharedCommunicator = initialize(std::move(initData));

            if (addFacets)
            {
                // Add all facets created on shared communicator to the IceBox communicator
                // but renamed <prefix>.<facet-name>, except for the Process facet which is
                // never added.

                FacetMap facets = _sharedCommunicator->findAllAdminFacets();
                for (const auto& facet : facets)
                {
                    if (facet.first != "Process")
                    {
                        _communicator->addAdminFacet(facet.second, facetNamePrefix + facet.first);
                    }
                }
            }
        }

        //
        // Start the services.
        //
        for (const auto& r : servicesInfo)
        {
            start(r.name, r.entryPoint, r.args);
        }

        // Refresh module list after loading dynamic libraries if stack trace collection is enabled.
        if (printStackTraces(_communicator->getProperties()))
        {
            try
            {
                LocalException::ice_enableStackTraceCollection();
            }
            catch (const std::exception& ex)
            {
                Warning out(_communicator->getLogger());
                out << "Cannot enable/refresh stack trace collection:\n" << ex;
                out << "\nYou can turn off this warning by setting Ice.PrintStackTraces=0";
            }
        }

        //
        // Start Admin (if enabled).
        //
        _communicator->addAdminFacet(shared_from_this(), "IceBox.ServiceManager");
        _communicator->getAdmin();

        //
        // We may want to notify external scripts that the services
        // have started and that IceBox is "ready".
        // This is done by defining the property IceBox.PrintServicesReady=bundleName
        //
        // bundleName is whatever you choose to call this set of
        // services. It will be echoed back as "bundleName ready".
        //
        // This must be done after start() has been invoked on the
        // services.
        //
        string bundleName = properties->getIceProperty("IceBox.PrintServicesReady");
        if (!bundleName.empty())
        {
            consoleOut << bundleName << " ready" << endl;
        }
    }
    catch (const FailureException& ex)
    {
        Error out(_logger);
        out << ex.what();
        stopAll();
        return false;
    }
    catch (const Exception& ex)
    {
        Error out(_logger);
        out << "ServiceManager: " << ex;
        stopAll();
        return false;
    }
    catch (const std::exception& ex)
    {
        Error out(_logger);
        out << "ServiceManager: " << ex.what();
        stopAll();
        return false;
    }
    catch (...)
    {
        Error out(_logger);
        out << "ServiceManager: unknown exception";
        stopAll();
        return false;
    }

    return true;
}

void
IceBox::ServiceManagerI::stop()
{
    stopAll();
}

void
IceBox::ServiceManagerI::start(const string& service, const string& entryPoint, const StringSeq& args)
{
    lock_guard<mutex> lock(_mutex);

    //
    // Load the entry point.
    //
    IceInternal::DynamicLibrary library;
    IceInternal::DynamicLibrary::symbol_type sym = library.loadEntryPoint(entryPoint, false);
    if (sym == nullptr)
    {
        ostringstream os;
        os << "ServiceManager: unable to load entry point '" << entryPoint << "'";
        const string& msg = library.getErrorMessage();
        if (!msg.empty())
        {
            os << ": " + msg;
        }
        throw FailureException(__FILE__, __LINE__, os.str());
    }

    ServiceInfo info;
    info.name = service;
    info.status = Stopped;
    info.args = args;

    //
    // If IceBox.UseSharedCommunicator.<name> is not defined, create
    // a communicator for the service. The communicator inherits
    // from the shared communicator properties. If it's defined
    // add the service properties to the shared communicator
    // property set.
    //
    Ice::CommunicatorPtr communicator;

    if (_communicator->getProperties()->getIcePropertyAsInt("IceBox.UseSharedCommunicator." + service) > 0)
    {
        assert(_sharedCommunicator);
        communicator = _sharedCommunicator;
    }
    else
    {
        try
        {
            //
            // Create the service properties. We use the communicator properties as the default
            // properties if IceBox.InheritProperties is set.
            //
            InitializationData initData;
            initData.properties = createServiceProperties(service);

            if (!info.args.empty())
            {
                //
                // Create the service properties with the given service arguments. This should
                // read the service config file if it's specified with --Ice.Config.
                //
                initData.properties = createProperties(info.args, initData.properties);

                //
                // Next, parse the service "<service>.*" command line options (the Ice command
                // line options were parsed by the createProperties above)
                //
                info.args = initData.properties->parseCommandLineOptions(service, info.args);
            }

            //
            // If Admin is enabled on the IceBox communicator, for each service that does not set
            // Ice.Admin.Enabled, we set Ice.Admin.Enabled=1 to have this service create facets; then
            // we add these facets to the IceBox Admin object as IceBox.Service.<service>.<facet>.
            //
            const string serviceFacetNamePrefix = "IceBox.Service." + service + ".";
            bool addFacets = configureAdmin(initData.properties, serviceFacetNamePrefix);

            info.communicator = initialize(std::move(initData));
            communicator = info.communicator;

            if (addFacets)
            {
                // Add all facets created on the service communicator to the IceBox communicator
                // but renamed IceBox.Service.<service>.<facet-name>, except for the Process facet
                // which is never added

                FacetMap facets = communicator->findAllAdminFacets();
                for (const auto& facet : facets)
                {
                    if (facet.first != "Process")
                    {
                        _communicator->addAdminFacet(facet.second, serviceFacetNamePrefix + facet.first);
                    }
                }
            }
        }
        catch (const Exception& ex)
        {
            LoggerOutputBase s;
            s << "ServiceManager: exception while starting service " << service << ":\n";
            s << ex;

            throw FailureException(__FILE__, __LINE__, s.str());
        }
    }

    try
    {
        //
        // Invoke the factory function.
        //

        auto factory = reinterpret_cast<ServiceFactory>(sym);
        try
        {
            info.service = ServicePtr(factory(_communicator));
        }
        catch (const FailureException&)
        {
            throw;
        }
        catch (const Exception& ex)
        {
            LoggerOutputBase s;
            s << "ServiceManager: exception in entry point '" + entryPoint + "' for service " << info.name << ":\n";
            s << ex;

            throw FailureException(__FILE__, __LINE__, s.str());
        }
        catch (...)
        {
            ostringstream s;
            s << "ServiceManager: unknown exception in entry point '" + entryPoint + "' for service " << info.name;

            throw FailureException(__FILE__, __LINE__, s.str());
        }

        //
        // Start the service.
        //
        try
        {
            info.service->start(info.name, communicator, info.args);

            //
            // There is no need to notify the observers since the 'start all'
            // (that indirectly calls this function) occurs before the creation of
            // the Server Admin object, and before the activation of the main
            // object adapter (so before any observer can be registered)
            //
        }
        catch (const FailureException&)
        {
            throw;
        }
        catch (const Exception& ex)
        {
            LoggerOutputBase s;
            s << "ServiceManager: exception while starting service " << info.name << ":\n";
            s << ex;

            throw FailureException(__FILE__, __LINE__, s.str());
        }
        catch (...)
        {
            ostringstream s;
            s << "ServiceManager: unknown exception while starting service " << info.name;

            throw FailureException(__FILE__, __LINE__, s.str());
        }

        info.status = Started;
        _services.push_back(info);
    }
    catch (const Exception&)
    {
        if (info.communicator)
        {
            destroyServiceCommunicator(info.name, info.communicator);
        }

        throw;
    }
}

void
IceBox::ServiceManagerI::stopAll()
{
    unique_lock<mutex> lock(_mutex);

    //
    // First wait for any active startService/stopService calls to complete.
    //
    _conditionVariable.wait(lock, [this] { return !_pendingStatusChanges; });

    //
    // Services are stopped in the reverse order from which they are started.
    //

    vector<string> stoppedServices;

    //
    // First, for each service, we call stop on the service.
    //
    for (auto p = _services.rbegin(); p != _services.rend(); ++p)
    {
        ServiceInfo& info = *p;

        if (info.status == Started)
        {
            try
            {
                info.service->stop();
                info.status = Stopped;
                stoppedServices.push_back(info.name);
            }
            catch (const Exception& ex)
            {
                Warning out(_logger);
                out << "ServiceManager: exception while stopping service " << info.name << ":\n";
                out << ex;
            }
            catch (...)
            {
                Warning out(_logger);
                out << "ServiceManager: unknown exception while stopping service " << info.name;
            }
        }
    }

    for (auto p = _services.rbegin(); p != _services.rend(); ++p)
    {
        ServiceInfo& info = *p;

        if (info.communicator)
        {
            try
            {
                info.communicator->shutdown();
                info.communicator->waitForShutdown();
            }
            catch (const CommunicatorDestroyedException&)
            {
                //
                // Ignore, the service might have already destroyed
                // the communicator for its own reasons.
                //
            }
            catch (const Exception& ex)
            {
                Warning out(_logger);
                out << "ServiceManager: exception while stopping service " << info.name << ":\n";
                out << ex;
            }
        }

        // Release the service and then the service communicator. The order is important, the service must be released
        // before destroying the communicator so that the communicator leak detector doesn't report potential leaks.
        info.service = nullptr;
        if (info.communicator)
        {
            removeAdminFacets("IceBox.Service." + info.name + ".");

            info.communicator->destroy();
            info.communicator = nullptr;
        }
    }

    if (_sharedCommunicator)
    {
        removeAdminFacets("IceBox.SharedCommunicator.");

        _sharedCommunicator->destroy();
        _sharedCommunicator = nullptr;
    }

    _services.clear();

    servicesStopped(stoppedServices, _observers);
}

function<void(exception_ptr)>
IceBox::ServiceManagerI::makeObserverCompletedCallback(ServiceObserverPrx observer)
{
    weak_ptr<ServiceManagerI> self = shared_from_this();
    return [self, observer = std::move(observer)](exception_ptr ex)
    {
        auto s = self.lock();
        if (s)
        {
            s->observerCompleted(observer, ex);
        }
    };
}
void
IceBox::ServiceManagerI::servicesStarted(const vector<string>& services, const set<ServiceObserverPrx>& observers)
{
    if (services.size() > 0)
    {
        for (const auto& observer : observers)
        {
            observer->servicesStartedAsync(services, nullptr, makeObserverCompletedCallback(observer));
        }
    }
}

void
IceBox::ServiceManagerI::servicesStopped(const vector<string>& services, const set<ServiceObserverPrx>& observers)
{
    if (services.size() > 0)
    {
        for (const auto& observer : observers)
        {
            observer->servicesStoppedAsync(services, nullptr, makeObserverCompletedCallback(observer));
        }
    }
}

void
IceBox::ServiceManagerI::observerRemoved(const ServiceObserverPrx& observer, exception_ptr err)
{
    if (_traceServiceObserver >= 1)
    {
        try
        {
            rethrow_exception(err);
        }
        catch (const CommunicatorDestroyedException&)
        {
            // Can occur during shutdown. The observer notification has been sent, but the communicator was destroyed
            // before the reply was received. We do not log a message for this exception.
        }
        catch (const exception& ex)
        {
            Trace out(_logger, "IceBox.ServiceObserver");
            out << "Removed service observer " << observer << "\nafter catching " << ex.what();
        }
    }
}

Ice::PropertiesPtr
IceBox::ServiceManagerI::createServiceProperties(const string& service)
{
    // We don't want to clone the properties object as we don't want to copy the opt-in prefix list.
    // NOTE: We always enable the "IceStorm" prefix as there's currently no way  to distinguish it.
    PropertiesPtr properties = make_shared<Properties>("IceStorm");
    PropertiesPtr communicatorProperties = _communicator->getProperties();
    if (communicatorProperties->getIcePropertyAsInt("IceBox.InheritProperties") > 0)
    {
        for (const auto& p : communicatorProperties->getPropertiesForPrefix(""))
        {
            // Inherit all except IceBox. and Ice.Admin. properties
            if (p.first.find("IceBox.") != 0 && p.first.find("Ice.Admin.") != 0)
            {
                properties->setProperty(p.first, p.second);
            }
        }
    }

    string programName = communicatorProperties->getIceProperty("Ice.ProgramName");
    if (programName.empty())
    {
        properties->setProperty("Ice.ProgramName", service);
    }
    else
    {
        properties->setProperty("Ice.ProgramName", programName + "-" + service);
    }
    return properties;
}

void
ServiceManagerI::observerCompleted(const ServiceObserverPrx& observer, exception_ptr ex)
{
    lock_guard<mutex> lock(_mutex);
    // It's possible to remove several times the same observer, e.g. multiple concurrent
    // requests that fail
    auto p = _observers.find(observer);
    if (p != _observers.end())
    {
        _observers.erase(p);
        observerRemoved(observer, ex);
    }
}

void
IceBox::ServiceManagerI::destroyServiceCommunicator(const string& service, const CommunicatorPtr& communicator)
{
    try
    {
        communicator->shutdown();
        communicator->waitForShutdown();
    }
    catch (const CommunicatorDestroyedException&)
    {
        //
        // Ignore, the service might have already destroyed
        // the communicator for its own reasons.
        //
    }
    catch (const Exception& ex)
    {
        Warning out(_logger);
        out << "ServiceManager: exception in shutting down communicator for service " << service << ":\n";
        out << ex;
    }

    removeAdminFacets("IceBox.Service." + service + ".");
    communicator->destroy();
}

bool
IceBox::ServiceManagerI::configureAdmin(const PropertiesPtr& properties, const string& prefix)
{
    if (_adminEnabled && properties->getIceProperty("Ice.Admin.Enabled").empty())
    {
        StringSeq facetNames;
        for (const auto& p : _adminFacetFilter)
        {
            if (p.find(prefix) == 0) // found
            {
                facetNames.push_back(p.substr(prefix.size()));
            }
        }

        if (_adminFacetFilter.empty() || !facetNames.empty())
        {
            properties->setProperty("Ice.Admin.Enabled", "1");

            if (!facetNames.empty())
            {
                // TODO: need joinString with escape!
                properties->setProperty("Ice.Admin.Facets", IceInternal::joinString(facetNames, " "));
            }
            return true;
        }
    }
    return false;
}

void
IceBox::ServiceManagerI::removeAdminFacets(const string& prefix)
{
    try
    {
        FacetMap facets = _communicator->findAllAdminFacets();

        for (const auto& facet : facets)
        {
            if (facet.first.find(prefix) == 0)
            {
                _communicator->removeAdminFacet(facet.first);
            }
        }
    }
    catch (const CommunicatorDestroyedException&)
    {
        // Ignored
    }
    catch (const ObjectAdapterDestroyedException&)
    {
        // Ignored
    }
}
