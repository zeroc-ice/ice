// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <IceUtil/Options.h>
#include <Ice/Ice.h>
#include <Ice/DynamicLibrary.h>
#include <Ice/SliceChecksums.h>
#include <IceBox/ServiceManagerI.h>

using namespace Ice;
using namespace IceBox;
using namespace std;

typedef IceBox::Service* (*SERVICE_FACTORY)(CommunicatorPtr);

namespace 
{

template<class T>
class AMICallback : public T
{
public:
    
    AMICallback(const ServiceManagerIPtr& serviceManager, const ServiceObserverPrx& observer) :
        _serviceManager(serviceManager),
        _observer(observer)
    {
    }
    
    virtual void ice_response()
    {
        // ok, success
    }
    
    virtual void ice_exception(const Ice::Exception& ex)
    {
        //
        // Drop this observer
        //
        _serviceManager->removeObserver(_observer, ex);
    }
    
private:
    ServiceManagerIPtr _serviceManager;
    ServiceObserverPrx _observer;
};

class PropertiesAdminI : public PropertiesAdmin
{
public:
    
    PropertiesAdminI(const PropertiesPtr& properties) :
        _properties(properties)
    {
    }
    
    virtual string getProperty(const string& name, const Current&)
    {
        return _properties->getProperty(name);
    }

    virtual PropertyDict getPropertiesForPrefix(const string& prefix, const Current&)
    {
        return _properties->getPropertiesForPrefix(prefix);
    }
    
private:

    const PropertiesPtr _properties; 
};

}

IceBox::ServiceManagerI::ServiceManagerI(CommunicatorPtr communicator, int& argc, char* argv[]) : 
    _communicator(communicator),
    _pendingStatusChanges(false),
    _traceServiceObserver(0)
{ 
    _logger = _communicator->getLogger();

    _traceServiceObserver = _communicator->getProperties()->getPropertyAsInt("IceBox.Trace.ServiceObserver");

    for(int i = 1; i < argc; i++)
    {
        _argv.push_back(argv[i]);
    }
}

IceBox::ServiceManagerI::~ServiceManagerI()
{
}

SliceChecksumDict
IceBox::ServiceManagerI::getSliceChecksums(const Current&) const
{
    return sliceChecksums();
}

void
IceBox::ServiceManagerI::startService(const string& name, const Current&)
{
    ServiceInfo info;
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);

        //
        // Search would be more efficient if services were contained in
        // a map, but order is required for shutdown.
        //
        vector<ServiceInfo>::iterator p;
        for(p = _services.begin(); p != _services.end(); ++p)
        {
            if(p->name == name)
            {
                if(p->status != Stopped)
                {
                    throw AlreadyStartedException();
                }
                p->status = Starting;
                info = *p;
                break;
            }
        }
        if(p == _services.end())
        {
            throw NoSuchServiceException();
        }
        _pendingStatusChanges = true;
    }

    bool started = false;
    try
    {
        info.service->start(name, info.communicator == 0 ? _communicator : info.communicator, info.args);
        started = true;
    }
    catch(const Ice::Exception& ex)
    {
        Warning out(_logger);
        out << "ServiceManager: exception in start for service " << info.name << ":\n";
        out << ex;
    }
    catch(...)
    {
        Warning out(_logger);
        out << "ServiceManager: unknown exception in start for service " << info.name;
    }
    
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);

        vector<ServiceInfo>::iterator p;
        for(p = _services.begin(); p != _services.end(); ++p)
        {
            if(p->name == name)
            {
                if(started)
                {
                    p->status = Started;

                    vector<string> services;
                    services.push_back(name);
                    servicesStarted(services, _observers);
                }
                else
                {
                    p->status = Stopped;
                }
                break;
            }
        }
        _pendingStatusChanges = false;
        notifyAll();
    }
}

void
IceBox::ServiceManagerI::stopService(const string& name, const Current&)
{
    ServiceInfo info;
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);

        //
        // Search would be more efficient if services were contained in
        // a map, but order is required for shutdown.
        //
        vector<ServiceInfo>::iterator p;
        for(p = _services.begin(); p != _services.end(); ++p)
        {
            if(p->name == name)
            {
                if(p->status != Started)
                {
                    throw AlreadyStoppedException();
                }
                p->status = Stopping;
                info = *p;
                break;
            }
        }
        if(p == _services.end())
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
    catch(const Ice::Exception& ex)
    {
        Warning out(_logger);
        out << "ServiceManager: exception in stop for service " << info.name << ":\n";
        out << ex;
    }
    catch(...)
    {
        Warning out(_logger);
        out << "ServiceManager: unknown exception in stop for service " << info.name;
    }

    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);

        vector<ServiceInfo>::iterator p;
        for(p = _services.begin(); p != _services.end(); ++p)
        {
            if(p->name == name)
            {
                if(stopped)
                {
                    p->status = Stopped;

                    vector<string> services;
                    services.push_back(name);
                    servicesStopped(services, _observers);
                }
                else
                {
                    p->status = Started;
                }
                break;
            }
        }
        _pendingStatusChanges = false;
        notifyAll();
    }
}

void
IceBox::ServiceManagerI::addObserver(const ServiceObserverPrx& observer, const Ice::Current&)
{
    //
    // Null observers and duplicate registrations are ignored
    //

    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
    if(observer != 0 && _observers.insert(observer).second)
    {
        if(_traceServiceObserver >= 1)
        {
            Trace out(_logger, "IceBox.ServiceObserver");
            out << "Added service observer " << _communicator->proxyToString(observer);
        } 

        vector<string> activeServices;
        for(vector<ServiceInfo>::iterator p = _services.begin(); p != _services.end(); ++p)
        {
            const ServiceInfo& info = *p;
            if(info.status == Started)
            {
                activeServices.push_back(info.name);
            }
        }
       
        if(activeServices.size() > 0)
        {
            observer->servicesStarted_async(new AMICallback<AMI_ServiceObserver_servicesStarted>(this, observer),
                                            activeServices);
        }
    }
}

void
IceBox::ServiceManagerI::removeObserver(const ServiceObserverPrx& observer, const Ice::Exception& ex)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);

    //
    // It's possible to remove several times the same observer, e.g. multiple concurrent
    // requests that fail
    //
    
    set<ServiceObserverPrx>::iterator p = _observers.find(observer);
    if(p != _observers.end())
    {
        ServiceObserverPrx observer = *p;
        _observers.erase(p);
        observerRemoved(observer, ex);
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
        ServiceManagerPtr obj = this;
        PropertiesPtr properties = _communicator->getProperties();

        //
        // Create an object adapter. Services probably should NOT share
        // this object adapter, as the endpoint(s) for this object adapter
        // will most likely need to be firewalled for security reasons.
        //
        ObjectAdapterPtr adapter;
        if(properties->getProperty("IceBox.ServiceManager.Endpoints") != "")
        {
            adapter = _communicator->createObjectAdapter("IceBox.ServiceManager");

            Identity identity;
            identity.category = properties->getPropertyWithDefault("IceBox.InstanceName", "IceBox");
            identity.name = "ServiceManager";
            adapter->add(obj, identity);
        }

        //
        // Load and start the services defined in the property set
        // with the prefix "IceBox.Service.". These properties should
        // have the following format:
        //
        // IceBox.Service.Foo=entry_point [args]
        //
        // We load the services specified in IceBox.LoadOrder first,
        // then load any remaining services.
        //
        const string prefix = "IceBox.Service.";
        PropertyDict services = properties->getPropertiesForPrefix(prefix);
        PropertyDict::iterator p;
        StringSeq loadOrder = properties->getPropertyAsList("IceBox.LoadOrder");
        for(StringSeq::const_iterator q = loadOrder.begin(); q != loadOrder.end(); ++q)
        {
            p = services.find(prefix + *q);
            if(p == services.end())
            {
                FailureException ex(__FILE__, __LINE__);
                ex.reason = "ServiceManager: no service definition for `" + *q + "'";
                throw ex;
            }
            load(*q, p->second);
            services.erase(p);
        }
        for(p = services.begin(); p != services.end(); ++p)
        {
            string name = p->first.substr(prefix.size());
            load(name, p->second);
        }

        //
        // We may want to notify external scripts that the services
        // have started. This is done by defining the property:
        //
        // IceBox.PrintServicesReady=bundleName
        //
        // Where bundleName is whatever you choose to call this set of
        // services. It will be echoed back as "bundleName ready".
        //
        // This must be done after start() has been invoked on the
        // services.
        //
        string bundleName = properties->getProperty("IceBox.PrintServicesReady");
        if(!bundleName.empty())
        {
            cout << bundleName << " ready" << endl;
        }

        //
        // Register "this" as a facet to the Admin object, and then create
        // Admin object
        //
        try
        {
            _communicator->addAdminFacet(this, "IceBox.ServiceManager");

            //
            // Add a Properties facet for each service
            // 
            for(vector<ServiceInfo>::iterator r = _services.begin(); r != _services.end(); ++r)
            {
                const ServiceInfo& info = *r;
                CommunicatorPtr communicator = info.communicator != 0 ? info.communicator : _sharedCommunicator;
                _communicator->addAdminFacet(new PropertiesAdminI(communicator->getProperties()),
                                             "IceBox.Service." + info.name + ".Properties");
            }
          
            _communicator->getAdmin();
        }
        catch(const ObjectAdapterDeactivatedException&)
        {
            //
            // Expected if the communicator has been shutdown.
            //
        }

        if(adapter)
        {
            try
            {
                adapter->activate();
            }
            catch(const ObjectAdapterDeactivatedException&)
            {
                //
                // Expected if the communicator has been shutdown.
                //
            }
        }
    }
    catch(const FailureException& ex)
    {
        Error out(_logger);
        out << ex.reason;
        stopAll();
        return false;
    }
    catch(const Exception& ex)
    {
        Error out(_logger);
        out << "ServiceManager: " << ex;
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
IceBox::ServiceManagerI::load(const string& name, const string& value)
{
    //
    // Separate the entry point from the arguments.
    //
    string entryPoint;
    StringSeq args;
    string::size_type pos = value.find_first_of(" \t\n");
    if(pos == string::npos)
    {
        entryPoint = value;
    }
    else
    {
        entryPoint = value.substr(0, pos);
        try
        {
            args = IceUtilInternal::Options::split(value.substr(pos + 1));
        }
        catch(const IceUtilInternal::BadOptException& ex)
        {
            FailureException e(__FILE__, __LINE__);
            e.reason = "ServiceManager: invalid arguments for service `" + name + "':\n" + ex.reason;
            throw e;        
        }
    }
    start(name, entryPoint, args);
}

void
IceBox::ServiceManagerI::start(const string& service, const string& entryPoint, const StringSeq& args)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);

    //
    // Create the service property set from the service arguments and
    // the server arguments. The service property set will be used to
    // create a new communicator, or will be added to the shared
    // communicator, depending on the value of the
    // IceBox.UseSharedCommunicator property.
    //
    ServiceInfo info;
    info.name = service;
    info.status = Stopped;
    StringSeq::size_type j;
    for(j = 0; j < args.size(); j++)
    {
        info.args.push_back(args[j]);
    }
    for(j = 0; j < _argv.size(); j++)
    {
        if(_argv[j].find("--" + service + ".") == 0)
        {
            info.args.push_back(_argv[j]);
        }
    }

    //
    // Load the entry point.
    //
    IceInternal::DynamicLibraryPtr library = new IceInternal::DynamicLibrary();
    IceInternal::DynamicLibrary::symbol_type sym = library->loadEntryPoint(entryPoint, false);
    if(sym == 0)
    {
        string msg = library->getErrorMessage();
        FailureException ex(__FILE__, __LINE__);
        ex.reason = "ServiceManager: unable to load entry point `" + entryPoint + "'";
        if(!msg.empty())
        {
            ex.reason += ": " + msg;
        }
        throw ex;
    }

    //
    // Invoke the factory function.
    //
    SERVICE_FACTORY factory = (SERVICE_FACTORY)sym;
    try
    {
        info.service = factory(_communicator);
    }
    catch(const Exception& ex)
    {
        FailureException e(__FILE__, __LINE__);
        e.reason = "ServiceManager: exception in entry point `" + entryPoint + "': " + ex.ice_name();
        throw e;
    }
    catch(...)
    {
        FailureException e(__FILE__, __LINE__);
        e.reason = "ServiceManager: unknown exception in entry point `" + entryPoint + "'";
        throw e;
    }

    //
    // Invoke Service::start().
    //
    try
    {
        //
        // If Ice.UseSharedCommunicator.<name> is not defined, create
        // a communicator for the service. The communicator inherits
        // from the shared communicator properties. If it's defined
        // add the service properties to the shared commnunicator
        // property set.
        //
        Ice::CommunicatorPtr communicator;
        if(_communicator->getProperties()->getPropertyAsInt("IceBox.UseSharedCommunicator." + service) > 0)
        {
            if(!_sharedCommunicator)
            {
                Ice::StringSeq dummy = Ice::StringSeq();
                _sharedCommunicator = createCommunicator("", dummy);
            }
            communicator = _sharedCommunicator;

            PropertiesPtr properties = _sharedCommunicator->getProperties();

            PropertiesPtr svcProperties = createProperties(info.args, properties);

            //
            // Erase properties from the shared communicator which don't exist in the
            // service properties (which include the shared communicator properties
            // overriden by the service properties).
            //
            PropertyDict allProps = properties->getPropertiesForPrefix("");
            for(PropertyDict::iterator p = allProps.begin(); p != allProps.end(); ++p)
            {
                if(svcProperties->getProperty(p->first) == "")
                {
                    properties->setProperty(p->first, "");
                }
            }

            //
            // Add the service properties to the shared communicator properties.
            //
            PropertyDict props = svcProperties->getPropertiesForPrefix("");
            for(PropertyDict::const_iterator q = props.begin(); q != props.end(); ++q)
            {
                properties->setProperty(q->first, q->second);
            }
            
            //
            // Parse <service>.* command line options (the Ice command line options 
            // were parsed by the createProperties above)
            //
            info.args = properties->parseCommandLineOptions(service, info.args);
        }
        else
        {       
            info.communicator = createCommunicator(service, info.args);
            communicator = info.communicator;
        }

        //
        // Start the service.
        //
        try
        {
            info.service->start(service, communicator, info.args);
            info.status = Started;

            //
            // There is no need to notify the observers since the 'start all'
            // (that indirectly calls this function) occurs before the creation of 
            // the Server Admin object, and before the activation of the main 
            // object adapter (so before any observer can be registered)
            //
        }
        catch(...)
        {
            if(info.communicator)
            {
                try
                {
                    info.communicator->shutdown();
                    info.communicator->waitForShutdown();
                }
                catch(const Ice::CommunicatorDestroyedException&)
                {
                    //
                    // Ignore, the service might have already destroyed
                    // the communicator for its own reasons.
                    //
                }
                catch(const Ice::Exception& ex)
                {
                    Warning out(_logger);
                    out << "ServiceManager: exception in shutting down communicator for service " << service << ":\n";
                    out << ex;
                }

                try
                {
                    info.communicator->destroy();
                    info.communicator = 0;
                }
                catch(const Exception& ex)
                {
                    Warning out(_logger);
                    out << "ServiceManager: exception in shutting down communicator for service " << service << ":\n";
                    out << ex;
                }
            }
            throw;
        }

        info.library = library;
        _services.push_back(info);
    }
    catch(const FailureException&)
    {
        throw;
    }
    catch(const Exception& ex)
    {
        ostringstream s;
        s << "ServiceManager: exception while starting service " << service << ":\n";
        s << ex;

        FailureException e(__FILE__, __LINE__);
        e.reason = s.str();
        throw e;
    }
}

void
IceBox::ServiceManagerI::stopAll()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);

    //
    // First wait for any active startService/stopService calls to complete.
    //
    while(_pendingStatusChanges)
    {
        wait();
    }

    //
    // Services are stopped in the reverse order from which they are started.
    //
    vector<ServiceInfo>::reverse_iterator p;

    vector<string> stoppedServices;

    //
    // First, for each service, we call stop on the service and flush its database environment to 
    // the disk.
    //
    for(p = _services.rbegin(); p != _services.rend(); ++p)
    {
        ServiceInfo& info = *p;

        if(info.status == Started)
        {
            try
            {
                info.service->stop();
                info.status = Stopped;
                stoppedServices.push_back(info.name);
            }
            catch(const Ice::Exception& ex)
            {
                Warning out(_logger);
                out << "ServiceManager: exception in stop for service " << info.name << ":\n";
                out << ex;
            }
            catch(...)
            {
                Warning out(_logger);
                out << "ServiceManager: unknown exception in stop for service " << info.name;
            }
        }
    }

    for(p = _services.rbegin(); p != _services.rend(); ++p)
    {
        ServiceInfo& info = *p;
        
        try
        {
            _communicator->removeAdminFacet("IceBox.Service." + info.name + ".Properties");
        }
        catch(const LocalException&)
        {
            // Ignored
        }

        if(info.communicator)
        {
            try
            {
                info.communicator->shutdown();
                info.communicator->waitForShutdown();
            }
            catch(const Ice::CommunicatorDestroyedException&)
            {
                //
                // Ignore, the service might have already destroyed
                // the communicator for its own reasons.
                //
            }
            catch(const Ice::Exception& ex)
            {
                Warning out(_logger);
                out << "ServiceManager: exception in stop for service " << info.name << ":\n";
                out << ex;
            }
        }

        //
        // Release the service, the service communicator and then the library. The order is important, 
        // the service must be released before destroying the communicator so that the communicator
        // leak detector doesn't report potential leaks, and the communicator must be destroyed before 
        // the library is released since the library will destroy its global state.
        //
        try
        {
            info.service = 0;
        }
        catch(const Exception& ex)
        {
            Warning out(_logger);
            out << "ServiceManager: exception in stop for service " << info.name << ":\n";
            out << ex;
        }
        catch(...)
        {
            Warning out(_logger);
            out << "ServiceManager: unknown exception in stop for service " << info.name;
        }

        if(info.communicator)
        {
            try
            {
                info.communicator->destroy();
                info.communicator = 0;
            }
            catch(const Exception& ex)
            {
                Warning out(_logger);
                out << "ServiceManager: exception in stop for service " << info.name << ":\n";
                out << ex;
            }
        }
        
        try
        {
            info.library = 0;
        }
        catch(const Exception& ex)
        {
            Warning out(_logger);
            out << "ServiceManager: exception in stop for service " << info.name << ":\n";
            out << ex;
        }
        catch(...)
        {
            Warning out(_logger);
            out << "ServiceManager: unknown exception in stop for service " << info.name;
        }
    }

    if(_sharedCommunicator)
    {
        try
        {
            _sharedCommunicator->destroy();
        }
        catch(const std::exception& ex)
        {
            Warning out(_logger);
            out << "ServiceManager: unknown exception while destroying shared communicator:\n" << ex.what();
        }
        _sharedCommunicator = 0;
    }

    _services.clear();

    servicesStopped(stoppedServices, _observers);
}

void
IceBox::ServiceManagerI::servicesStarted(const vector<string>& services, const set<ServiceObserverPrx>& observers)
{
    if(services.size() > 0)
    {
        for(set<ServiceObserverPrx>::const_iterator p = observers.begin(); p != observers.end(); ++p)
        {
            ServiceObserverPrx observer = *p;
            observer->servicesStarted_async(new AMICallback<AMI_ServiceObserver_servicesStarted>(this, observer),
                                            services);
        }
    }
}

void
IceBox::ServiceManagerI::servicesStopped(const vector<string>& services, const set<ServiceObserverPrx>& observers)
{
    if(services.size() > 0)
    {
        for(set<ServiceObserverPrx>::const_iterator p = observers.begin(); p != observers.end(); ++p)
        {
            ServiceObserverPrx observer = *p;
            observer->servicesStopped_async(new AMICallback<AMI_ServiceObserver_servicesStopped>(this, observer),
                                            services);
        }
    }
}

void
IceBox::ServiceManagerI::observerRemoved(const ServiceObserverPrx& observer, const std::exception& ex)
{ 
    if(_traceServiceObserver >= 1)
    {
        //
        // CommunicatorDestroyedException may occur during shutdown. The observer notification has
        // been sent, but the communicator was destroyed before the reply was received. We do not
        // log a message for this exception.
        //
        if(dynamic_cast<const CommunicatorDestroyedException*>(&ex) == 0)
        {
            Trace out(_logger, "IceBox.ServiceObserver");
            out << "Removed service observer " << _communicator->proxyToString(observer)
                << "\nafter catching " << ex.what();
        }
    } 
} 

Ice::CommunicatorPtr
IceBox::ServiceManagerI::createCommunicator(const string& service, Ice::StringSeq& args)
{
    PropertiesPtr communicatorProperties = _communicator->getProperties();

    //
    // Create the service properties. We use the communicator properties as the default
    // properties if IceBox.InheritProperties is set.
    //
    PropertiesPtr properties;
    if(communicatorProperties->getPropertyAsInt("IceBox.InheritProperties") > 0)
    {
        properties = communicatorProperties->clone();
        properties->setProperty("Ice.Admin.Endpoints", ""); // Inherit all except Ice.Admin.Endpoints!
    }
    else
    {
        properties = createProperties();
    }

    //
    // Set the default program name for the service properties. By default it's 
    // the IceBox program name + "-" + the service name, or just the IceBox 
    // program name if we're creating the shared communicator (service == "").
    //
    string programName = communicatorProperties->getProperty("Ice.ProgramName");
    if(service.empty())
    {
        if(programName.empty())
        {
            properties->setProperty("Ice.ProgramName", "SharedCommunicator");
        }
        else
        {
            properties->setProperty("Ice.ProgramName", programName + "-SharedCommunicator");
        }
    }
    else
    {
        if(programName.empty())
        {
            properties->setProperty("Ice.ProgramName", service);
        }
        else
        {
            properties->setProperty("Ice.ProgramName", programName + "-" + service);
        }
    }

    if(!args.empty())
    {
        //
        // Create the service properties with the given service arguments. This should
        // read the service config file if it's specified with --Ice.Config.
        //
        properties = createProperties(args, properties);
        
        if(!service.empty())
        {
            //
            // Next, parse the service "<service>.*" command line options (the Ice command 
            // line options were parsed by the createProperties above)
            //
            args = properties->parseCommandLineOptions(service, args);
        }
    }

    //
    // Remaining command line options are passed to the communicator. This is 
    // necessary for Ice plugin properties (e.g.: IceSSL).
    //
    InitializationData initData;
    initData.properties = properties;
    return initialize(args, initData);
}
