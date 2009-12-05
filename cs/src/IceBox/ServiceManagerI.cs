// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections;
using System.Collections.Generic;
using System.Threading;
using System.Diagnostics;

namespace IceBox
{

//
// NOTE: the class isn't final on purpose to allow users to eventually
// extend it.
//
class ServiceManagerI : ServiceManagerDisp_
{
    class AMIServicesStartedCallback : AMI_ServiceObserver_servicesStarted
    {
        public AMIServicesStartedCallback(ServiceManagerI serviceManager, ServiceObserverPrx observer)
        {
            _serviceManager = serviceManager;
            _observer = observer;
        }
  
        public override void ice_response()
        {
            // ok, success
        }
        
        public override void ice_exception(Ice.Exception ex)
        {
            //
            // Drop this observer
            //
            _serviceManager.removeObserver(_observer, ex);
        }

        private ServiceManagerI _serviceManager;
        private ServiceObserverPrx _observer;
    }
    
    //
    // TODO: would be nice to avoid the duplication AMIServicesStartedCallback/AMIServicesStoppedCallback
    //
    class AMIServicesStoppedCallback : AMI_ServiceObserver_servicesStopped
    {
        public AMIServicesStoppedCallback(ServiceManagerI serviceManager, ServiceObserverPrx observer)
        {
            _serviceManager = serviceManager;
            _observer = observer;
        }
  
        public override void ice_response()
        {
            // ok, success
        }
        
        public override void ice_exception(Ice.Exception ex)
        {
            //
            // Drop this observer
            //
            _serviceManager.removeObserver(_observer, ex);
        }

        private ServiceManagerI _serviceManager;
        private ServiceObserverPrx _observer;
    }

    public ServiceManagerI(Ice.Communicator communicator, string[] args)
    {
        _communicator = communicator;
        _logger = _communicator.getLogger();
        _argv = args;
        _traceServiceObserver = _communicator.getProperties().getPropertyAsInt("IceBox.Trace.ServiceObserver");

    }

    public override Dictionary<string, string>
    getSliceChecksums(Ice.Current current)
    {
        return Ice.SliceChecksums.checksums;
    }

    public override void
    startService(string name, Ice.Current current)
    {
        ServiceInfo info = new ServiceInfo();
        lock(this)
        {
            //
            // Search would be more efficient if services were contained in
            // a map, but order is required for shutdown.
            //
            int i;
            for(i = 0; i < _services.Count; ++i)
            {
                info = _services[i];
                if(info.name.Equals(name))
                {
                    if(_services[i].status != ServiceStatus.Stopped)
                    {
                        throw new AlreadyStartedException();
                    }
                    info.status = ServiceStatus.Starting;
                    _services[i] = info;
                    break;
                }
            }
            if(i == _services.Count)
            {
                throw new NoSuchServiceException();
            }
            _pendingStatusChanges = true;
        }

        bool started = false;
        try
        {
            info.service.start(info.name, info.communicator == null ? _sharedCommunicator : info.communicator, 
                               info.args);
            started = true;
        }
        catch(Exception e)
        {
            _logger.warning("ServiceManager: exception in start for service " + info.name + "\n" + 
                            e.ToString());
        }

        lock(this)
        {
            int i;
            for(i = 0; i < _services.Count; ++i)
            {
                info = _services[i];
                if(info.name.Equals(name))
                {
                    if(started)
                    {
                        info.status = ServiceStatus.Started;

                        List<string> services = new List<string>();
                        services.Add(name);
                        servicesStarted(services, _observers.Keys);
                    }
                    else
                    {
                        info.status = ServiceStatus.Stopped;
                    }
                    _services[i] = info;
                    break;
                }
            }
            _pendingStatusChanges = false;
            Monitor.PulseAll(this);
        }
    }

    public override void
    stopService(string name, Ice.Current current)
    {
        ServiceInfo info = new ServiceInfo();
        lock(this)
        {
            //
            // Search would be more efficient if services were contained in
            // a map, but order is required for shutdown.
            //
            int i;
            for(i = 0; i < _services.Count; ++i)
            {
                info = _services[i];
                if(info.name.Equals(name))
                {
                    if(info.status != ServiceStatus.Started)
                    {
                        throw new AlreadyStoppedException();
                    }
                    info.status = ServiceStatus.Stopping;
                    _services[i] = info;
                    break;
                }
            }
            if(i == _services.Count)
            {
                throw new NoSuchServiceException();
            }
            _pendingStatusChanges = true;
        }

        bool stopped = false;
        try
        {
            info.service.stop();
            stopped = true;
        }
        catch(Exception e)
        {
            _logger.warning("ServiceManager: exception in stop for service " + info.name + "\n" + 
                            e.ToString());
        }

        lock(this)
        {
            int i;
            for(i = 0; i < _services.Count; ++i)
            {
                info = _services[i];
                if(info.name.Equals(name))
                {
                    if(stopped)
                    {
                        info.status = ServiceStatus.Stopped;
                        
                        List<string> services = new List<string>();
                        services.Add(name);
                        servicesStopped(services, _observers.Keys);
                    }
                    else
                    {
                        info.status = ServiceStatus.Started;
                    }
                    _services[i] = info;
                    break;
                }
            }
            _pendingStatusChanges = false;
            Monitor.PulseAll(this);
        }
    }

    public override void
    addObserver(ServiceObserverPrx observer, Ice.Current current)
    {
        List<string> activeServices = new List<string>();

        //
        // Null observers and duplicate registrations are ignored
        //

        lock(this)
        {
            if(observer != null)
            {
                try
                {
                    _observers.Add(observer, true);
                }
                catch(ArgumentException)
                {
                    return;
                }
               
                if(_traceServiceObserver >= 1)
                {
                    _logger.trace("IceBox.ServiceObserver", 
                                  "Added service observer " + _communicator.proxyToString(observer));
                } 

                foreach(ServiceInfo info in _services)
                {
                    if(info.status == ServiceStatus.Started)
                    {
                        activeServices.Add(info.name);
                    }
                }
            }
        }

        if(activeServices.Count > 0)
        {
            observer.servicesStarted_async(new AMIServicesStartedCallback(this, observer),
                                           activeServices.ToArray());          
        }
    }

    public override void
    shutdown(Ice.Current current)
    {
        _communicator.shutdown();
    }

    public int
    run()
    {
        try
        {
            Ice.Properties properties = _communicator.getProperties();

            //
            // Create an object adapter. Services probably should NOT share
            // this object adapter, as the endpoint(s) for this object adapter
            // will most likely need to be firewalled for security reasons.
            //
            Ice.ObjectAdapter adapter = null;
            if(properties.getProperty("IceBox.ServiceManager.Endpoints").Length != 0)
            {
                adapter = _communicator.createObjectAdapter("IceBox.ServiceManager");

                Ice.Identity identity = new Ice.Identity();
                identity.category = properties.getPropertyWithDefault("IceBox.InstanceName", "IceBox");
                identity.name = "ServiceManager";
                adapter.add(this, identity);
            }

            //
            // Parse the property set with the prefix "IceBox.Service.". These
            // properties should have the following format:
            //
            // IceBox.Service.Foo=Package.Foo [args]
            //
            // We parse the service properties specified in IceBox.LoadOrder 
            // first, then the ones from remaining services.
            //
            string prefix = "IceBox.Service.";
            Dictionary<string, string> services = properties.getPropertiesForPrefix(prefix);
            string[] loadOrder = properties.getPropertyAsList("IceBox.LoadOrder");
            List<StartServiceInfo> servicesInfo = new List<StartServiceInfo>();
            for(int i = 0; i < loadOrder.Length; ++i)
            {
                if(loadOrder[i].Length > 0)
                {
                    string key = prefix + loadOrder[i];
                    string value = services[key];
                    if(value == null)
                    {
                        FailureException ex = new FailureException();
                        ex.reason = "ServiceManager: no service definition for `" + loadOrder[i] + "'";
                        throw ex;
                    }
                    servicesInfo.Add(new StartServiceInfo(loadOrder[i], value, _argv));
                    services.Remove(key);
                }
            }
            foreach(KeyValuePair<string, string> entry in services)
            {
                string name = entry.Key.Substring(prefix.Length);
                string value = entry.Value;
                servicesInfo.Add(new StartServiceInfo(name, value, _argv));
            }

            //
            // Check if some services are using the shared communicator in which
            // case we create the shared communicator now with a property set which
            // is the union of all the service properties (services which are using
            // the shared communicator).
            //
            if(properties.getPropertiesForPrefix("IceBox.UseSharedCommunicator.").Count > 0)
            {
                Ice.InitializationData initData = new Ice.InitializationData();
                initData.properties = createServiceProperties("SharedCommunicator");
                foreach(StartServiceInfo service in servicesInfo)
                {
                    if(properties.getPropertyAsInt("IceBox.UseSharedCommunicator." + service.name) <= 0)
                    {
                        continue;
                    }

                    //
                    // Load the service properties using the shared communicator properties as
                    // the default properties.
                    //
                    Ice.Properties svcProperties = Ice.Util.createProperties(ref service.args, initData.properties);

                    //
                    // Erase properties from the shared communicator which don't exist in the
                    // service properties (which include the shared communicator properties
                    // overriden by the service properties).
                    //
                    Dictionary<string, string> allProps = initData.properties.getPropertiesForPrefix("");
                    foreach(string key in allProps.Keys)
                    {
                        if(svcProperties.getProperty(key).Length == 0)
                        {
                            initData.properties.setProperty(key, "");
                        }
                    }
                    
                    //
                    // Add the service properties to the shared communicator properties.
                    //
                    foreach(KeyValuePair<string, string> entry in svcProperties.getPropertiesForPrefix(""))
                    {
                        initData.properties.setProperty(entry.Key, entry.Value);
                    }

                    //
                    // Parse <service>.* command line options (the Ice command line options 
                    // were parsed by the createProperties above)
                    //
                    service.args = initData.properties.parseCommandLineOptions(service.name, service.args);
                }
                _sharedCommunicator = Ice.Util.initialize(initData);
            }

            foreach(StartServiceInfo s in servicesInfo)
            {
                startService(s.name, s.entryPoint, s.args);
            }

            //
            // We may want to notify external scripts that the services
            // have started. This is done by defining the property:
            //
            // PrintServicesReady=bundleName
            //
            // Where bundleName is whatever you choose to call this set of
            // services. It will be echoed back as "bundleName ready".
            //
            // This must be done after start() has been invoked on the
            // services.
            //
            string bundleName = properties.getProperty("IceBox.PrintServicesReady");
            if(bundleName.Length > 0)
            {
                Console.Out.WriteLine(bundleName + " ready");
            }

            //
            // Don't move after the adapter activation. This allows
            // applications to wait for the service manager to be
            // reachable before sending a signal to shutdown the
            // 
            //
            Ice.Application.shutdownOnInterrupt();

            //
            // Register "this" as a facet to the Admin object and create Admin object
            //
            try
            {
                _communicator.addAdminFacet(this, "IceBox.ServiceManager");

                //
                // Add a Properties facet for each service
                // 
                foreach(ServiceInfo info in _services)
                {
                    Ice.Communicator communicator = info.communicator != null ? info.communicator : _sharedCommunicator;
                    _communicator.addAdminFacet(new PropertiesAdminI(communicator.getProperties()),
                                                "IceBox.Service." + info.name + ".Properties");
                }

                _communicator.getAdmin();
            }
            catch(Ice.ObjectAdapterDeactivatedException)
            {
                //
                // Expected if the communicator has been shutdown.
                //
            }

            //
            // Start request dispatching after we've started the services.
            //
            if(adapter != null)
            {
                try
                {
                    adapter.activate();
                }
                catch(Ice.ObjectAdapterDeactivatedException)
                {
                    //
                    // Expected if the communicator has been shutdown.
                    //
                }
            }

            _communicator.waitForShutdown();
            // XXX:
            //Ice.Application.defaultInterrupt();

            //
            // Invoke stop() on the services.
            //
            stopAll();
        }
        catch(FailureException ex)
        {
            _logger.error(ex.ToString());
            stopAll();
            return 1;
        }
        catch(Ice.LocalException ex)
        {
            _logger.error("ServiceManager: " + ex.ToString());
            stopAll();
            return 1;
        }
        catch(Exception ex)
        {
            _logger.error("ServiceManager: unknown exception\n" + ex.ToString());
            stopAll();
            return 1;
        }

        return 0;
    }

    private void
    startService(string service, string entryPoint, string[] args)
    {
        lock(this)
        {
            //
            // Instantiate the class.
            //
            ServiceInfo info = new ServiceInfo();
            info.name = service;
            info.status = ServiceStatus.Stopped;
            info.args = args;

            //
            // Retrieve the assembly name and the type.
            //
            string err = "ServiceManager: unable to load service '" + entryPoint + "': ";
            int sepPos = entryPoint.IndexOf(':');
            if(sepPos != -1)
            {
                if(entryPoint.Length > 3 &&
                   sepPos == 1 &&
                   System.Char.IsLetter(entryPoint[0]) &&
                   (entryPoint[2] == '\\' || entryPoint[2] == '/'))
                {
                    sepPos = entryPoint.IndexOf(':', 3);
                }
            }
            if(sepPos == -1)
            {
                FailureException e = new FailureException();
                e.reason = err + "invalid entry point format: " + entryPoint;
                throw e;
            }
        
            System.Reflection.Assembly serviceAssembly = null;
            string assemblyName = entryPoint.Substring(0, sepPos);
            try
            {
                //
                // First try to load the assemby using Assembly.Load which will succeed
                // if full name is configured or partial name has been qualified in config.
                // If that fails, try Assembly.LoadFrom() which will succeed if a file name
                // is configured or partial name is configured and DEVPATH is used.
                //
                try
                {
                    serviceAssembly = System.Reflection.Assembly.Load(assemblyName);
                }
                catch(System.Exception ex)
                {
                    try
                    {
                        serviceAssembly = System.Reflection.Assembly.LoadFrom(assemblyName);
                    }
                    catch(System.Exception)
                    {
                         throw ex;
                    }
                }
            }
            catch(System.Exception ex)
            {
                FailureException e = new FailureException(ex);
                e.reason = err + "unable to load assembly: " + assemblyName;
                throw e;
            }
            
            //
            // Instantiate the class.
            //
            string className = entryPoint.Substring(sepPos + 1);
            System.Type c = serviceAssembly.GetType(className);
            if(c == null)
            {
                FailureException e = new FailureException();
                e.reason = err + "GetType failed for '" + className + "'";
                throw e;
            }
        
            try
            {
                info.service = (Service)IceInternal.AssemblyUtil.createInstance(c);
                if(info.service == null)
                {
                    FailureException e = new FailureException();
                    e.reason = err + "Can't find constructor for '" + className + "'";
                    throw e;
                }
            }
            catch(System.InvalidCastException ex)
            {
                FailureException e = new FailureException(ex);
                e.reason = err + "InvalidCastException to IceBox.Service";
                throw e;
            }
            catch(System.UnauthorizedAccessException ex)
            {
                FailureException e = new FailureException(ex);
                e.reason = err + "UnauthorizedAccessException";
                throw e;
            }
            catch(System.Exception ex)
            {
                FailureException e = new FailureException(ex);
                e.reason = err + "System.Exception";
                throw e;
            }

            //
            // Invoke Service::start().
            //
            try
            {
                //
                // If IceBox.UseSharedCommunicator.<name> is defined, create a
                // communicator for the service. The communicator inherits
                // from the shared communicator properties. If it's not
                // defined, add the service properties to the shared
                // commnunicator property set.
                //
                Ice.Communicator communicator;
                if(_communicator.getProperties().getPropertyAsInt("IceBox.UseSharedCommunicator." + service) > 0)
                {
                    Debug.Assert(_sharedCommunicator != null);
                    communicator = _sharedCommunicator;
                }
                else
                {
                    //
                    // Create the service properties. We use the communicator properties as the default
                    // properties if IceBox.InheritProperties is set.
                    //
                    Ice.InitializationData initData = new Ice.InitializationData();
                    initData.properties = createServiceProperties(service);
                    if(info.args.Length > 0)
                    {
                        //
                        // Create the service properties with the given service arguments. This should
                        // read the service config file if it's specified with --Ice.Config.
                        //
                        initData.properties = Ice.Util.createProperties(ref info.args, initData.properties);

                        //
                        // Next, parse the service "<service>.*" command line options (the Ice command 
                        // line options were parsed by the createProperties above)
                        //
                        info.args = initData.properties.parseCommandLineOptions(service, info.args);
                    }

                    //
                    // Clone the logger to assign a new prefix.
                    //
                    initData.logger = _logger.cloneWithPrefix(initData.properties.getProperty("Ice.ProgramName"));
            
                    //
                    // Remaining command line options are passed to the communicator. This is 
                    // necessary for Ice plug-in properties (e.g.: IceSSL).
                    //
                    info.communicator = Ice.Util.initialize(ref info.args, initData);
                    communicator = info.communicator;
                }

                try
                {
                    info.service.start(service, communicator, info.args);
                    info.status = ServiceStatus.Started;
                }
                catch(Exception)
                {
                    if(info.communicator != null)
                    {
                        try
                        {
                            info.communicator.shutdown();
                            info.communicator.waitForShutdown();
                        }
                        catch(Ice.CommunicatorDestroyedException)
                        {
                            //
                            // Ignore, the service might have already destroyed
                            // the communicator for its own reasons.
                            //
                        }
                        catch(Exception e)
                        {
                            _logger.warning("ServiceManager: exception in shutting down communicator for service "
                                            + service + "\n" + e.ToString());
                        }
                    
                        try
                        {
                            info.communicator.destroy();
                        }
                        catch(Exception e)
                        {
                            _logger.warning("ServiceManager: exception in destroying communciator for service"
                                            + service + "\n" + e.ToString());
                        }
                    }
                    throw;
                }

                _services.Add(info);
            }
            catch(FailureException)
            {
                throw;
            }
            catch(Exception ex)
            {
                FailureException e = new FailureException(ex);
                e.reason = "ServiceManager: exception while starting service " + service + ": " + ex;
                throw e;
            }
        }
    }

    private void
    stopAll()
    {
        lock(this)
        {
            //
            // First wait for any active startService/stopService calls to complete.
            //
            while(_pendingStatusChanges)
            {
                Monitor.Wait(this);
            }

            //
            // First, for each service, we call stop on the service and flush its database environment to 
            // the disk. Services are stopped in the reverse order of which they were started.
            //
            _services.Reverse();
            List<string> stoppedServices = new List<string>();
            foreach(ServiceInfo info in _services)
            {
                if(info.status == ServiceStatus.Started)
                {
                    try
                    {
                        info.service.stop();
                        stoppedServices.Add(info.name);
                    }
                    catch(Exception e)
                    {
                        _logger.warning("IceBox.ServiceManager: exception in stop for service " + info.name + "\n" +
                                        e.ToString());
                    }
                }

                try
                {
                    _communicator.removeAdminFacet("IceBox.Service." + info.name + ".Properties");
                }
                catch(Ice.LocalException)
                {
                    // Ignored
                }

                if(info.communicator != null)
                {
                    try
                    {
                        info.communicator.shutdown();
                        info.communicator.waitForShutdown();
                    }
                    catch(Ice.CommunicatorDestroyedException)
                    {
                        //
                        // Ignore, the service might have already destroyed
                        // the communicator for its own reasons.
                        //
                    }
                    catch(Exception e)
                    {
                        _logger.warning("ServiceManager: exception in stop for service " + info.name + "\n" +
                                        e.ToString());
                    }
            
                    try
                    {
                        info.communicator.destroy();
                    }
                    catch(Exception e)
                    {
                        _logger.warning("ServiceManager: exception in stop for service " + info.name + "\n" +
                                        e.ToString());
                    }
                }
            }

            if(_sharedCommunicator != null)
            {
                try
                {
                    _sharedCommunicator.destroy();
                }
                catch(Exception e)
                {
                    _logger.warning("ServiceManager: unknown exception while destroying shared communicator:\n" +
                                    e.ToString());
                }
                _sharedCommunicator = null;
            }
            
            _services.Clear();
            servicesStopped(stoppedServices, _observers.Keys);
        }
    }

    private void
    servicesStarted(List<String> services, Dictionary<ServiceObserverPrx, bool>.KeyCollection observers)
    {
        //
        // Must be called with 'this' unlocked
        //

        if(services.Count > 0)
        {
            string[] servicesArray = services.ToArray();
          
            foreach(ServiceObserverPrx observer in observers)
            {
                AMI_ServiceObserver_servicesStarted cb = new AMIServicesStartedCallback(this, observer);
                observer.servicesStarted_async(cb, servicesArray);
            }
        }
    }

    private void
    servicesStopped(List<string> services, Dictionary<ServiceObserverPrx, bool>.KeyCollection observers)
    {
        //
        // Must be called with 'this' unlocked
        //

        if(services.Count > 0)
        {
            string[] servicesArray = services.ToArray();
            
            foreach(ServiceObserverPrx observer in observers)
            {
                AMI_ServiceObserver_servicesStopped cb = new AMIServicesStoppedCallback(this, observer);
                observer.servicesStopped_async(cb, servicesArray);
            }
        }
    }

    private void
    removeObserver(ServiceObserverPrx observer, Ice.Exception ex)
    {
        lock(this)
        {
            if(_observers.Remove(observer))
            {
                observerRemoved(observer, ex);
            }
        }
    }
    
    private void 
    observerRemoved(ServiceObserverPrx observer, System.Exception ex)
    {
        if(_traceServiceObserver >= 1)
        {
            //
            // CommunicatorDestroyedException may occur during shutdown. The observer notification has
            // been sent, but the communicator was destroyed before the reply was received. We do not
            // log a message for this exception.
            //
            if(!(ex is Ice.CommunicatorDestroyedException))
            {
                _logger.trace("IceBox.ServiceObserver",
                              "Removed service observer " + _communicator.proxyToString(observer)
                              + "\nafter catching " + ex.ToString());
            }
        } 
    } 
    
    private enum ServiceStatus
    {
        Stopping,
        Stopped,
        Starting,
        Started
    }

    struct ServiceInfo
    {
        public string name;
        public Service service;
        public Ice.Communicator communicator;
        public ServiceStatus status;
        public string[] args;
    }

    class StartServiceInfo
    {
        public StartServiceInfo(string service, string value, string[] serverArgs)
        {
            //
            // Separate the entry point from the arguments.
            //
            name = service;
            entryPoint = value;
            args = new string[0];
            int start = value.IndexOf(':');
            if(start != -1)
            {
                //
                // Find the whitespace.
                //
                int pos = value.IndexOf(' ', start);
                if(pos == -1)
                {
                    pos = value.IndexOf('\t', start);
                }
                if(pos == -1)
                {
                    pos = value.IndexOf('\n', start);
                }
                if(pos != -1)
                {
                    entryPoint = value.Substring(0, pos);
                    try
                    {
                        args = IceUtilInternal.Options.split(value.Substring(pos));
                    }
                    catch(IceUtilInternal.Options.BadQuote ex)
                    {
                        FailureException e = new FailureException();
                        e.reason = "ServiceManager: invalid arguments for service `" + name + "':\n" + ex.ToString();
                        throw e;
                    }
                }
            }

            if(serverArgs.Length > 0)
            {
                ArrayList l = new ArrayList();
                for(int j = 0; j < args.Length; j++)
                {
                    l.Add(args[j]);
                }
                for(int j = 0; j < serverArgs.Length; j++)
                {
                    if(serverArgs[j].StartsWith("--" + service + ".", StringComparison.Ordinal))
                    {
                        l.Add(serverArgs[j]);
                    }
                }
                args = (string[])l.ToArray(typeof(string));
            }
        }

        public string name;
        public string entryPoint;
        public string[] args;
    }

    class PropertiesAdminI : Ice.PropertiesAdminDisp_
    {
        public PropertiesAdminI(Ice.Properties properties)
        {
            _properties = properties;
        }

        public override string
        getProperty(string name, Ice.Current current)
        {
            return _properties.getProperty(name);
        }
        
        public override Dictionary<string, string>
        getPropertiesForPrefix(string name, Ice.Current current)
        {
            return _properties.getPropertiesForPrefix(name);
        }
        
        private Ice.Properties _properties;
    }

    private Ice.Properties
    createServiceProperties(String service)
    {
        Ice.Properties properties;
        Ice.Properties communicatorProperties = _communicator.getProperties();
        if(communicatorProperties.getPropertyAsInt("IceBox.InheritProperties") > 0)
        {
            properties = communicatorProperties.ice_clone_();
            properties.setProperty("Ice.Admin.Endpoints", ""); // Inherit all except Ice.Admin.Endpoints!
        }
        else
        {
            properties = Ice.Util.createProperties();
        }

        String programName = communicatorProperties.getProperty("Ice.ProgramName");
        if(programName.Length == 0)
        {
            properties.setProperty("Ice.ProgramName", service);
        }
        else
        {
            properties.setProperty("Ice.ProgramName", programName + "-" + service);
        }
        return properties;
    }

    private Ice.Communicator _communicator;
    private Ice.Communicator _sharedCommunicator = null;
    private Ice.Logger _logger;
    private string[] _argv; // Filtered server argument vector
    private List<ServiceInfo> _services = new List<ServiceInfo>();
    private bool _pendingStatusChanges = false;
    private Dictionary<ServiceObserverPrx, bool> _observers = new  Dictionary<ServiceObserverPrx, bool>();
    private int _traceServiceObserver = 0;
}

}
