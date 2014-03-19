// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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
    public ServiceManagerI(Ice.Communicator communicator, string[] args)
    {
        _communicator = communicator;
        _logger = _communicator.getLogger();
        _argv = args;
        _traceServiceObserver = _communicator.getProperties().getPropertyAsInt("IceBox.Trace.ServiceObserver");
    }

    public override Dictionary<string, string> getSliceChecksums(Ice.Current current)
    {
        return Ice.SliceChecksums.checksums;
    }

    public override void startService(string name, Ice.Current current)
    {
        ServiceInfo info = new ServiceInfo();
        _m.Lock();
        try
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
        finally
        {
            _m.Unlock();
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
            _logger.warning("ServiceManager: exception while starting service " + info.name + ":\n" + e.ToString());
        }

        _m.Lock();
        try
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
            _m.NotifyAll();
        }
        finally
        {
            _m.Unlock();
        }
    }

    public override void stopService(string name, Ice.Current current)
    {
        ServiceInfo info = new ServiceInfo();
        _m.Lock();
        try
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
        finally
        {
            _m.Unlock();
        }

        bool stopped = false;
        try
        {
            info.service.stop();
            stopped = true;
        }
        catch(Exception e)
        {
            _logger.warning("ServiceManager: exception while stopping service " + info.name + "\n" + e.ToString());
        }

        _m.Lock();
        try
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
            _m.NotifyAll();
        }
        finally
        {
            _m.Unlock();
        }
    }

    public override void addObserver(ServiceObserverPrx observer, Ice.Current current)
    {
        List<string> activeServices = new List<string>();

        //
        // Null observers and duplicate registrations are ignored
        //

        _m.Lock();
        try
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
        finally
        {
            _m.Unlock();
        }

        if(activeServices.Count > 0)
        {
            observer.begin_servicesStarted(activeServices.ToArray(), this.observerCompleted, null);
        }
    }

    public override void shutdown(Ice.Current current)
    {
        _communicator.shutdown();
    }

    public int run()
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
            // IceBox.Service.Foo=<assembly>:Package.Foo [args]
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
            // case we create the shared communicator now with a property set that
            // is the union of all the service properties (from services that use
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
                    // Remove properties from the shared property set that a service explicitly clears.
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
                    // were parsed by the call to createProperties above).
                    //
                    service.args = initData.properties.parseCommandLineOptions(service.name, service.args);
                }

                //
                // If Ice metrics are enabled on the IceBox communicator, we also enable them on the 
                // shared communicator.
                // 
                IceInternal.MetricsAdminI metricsAdmin = null;
                if(_communicator.getObserver() is IceInternal.CommunicatorObserverI)
                {
                    metricsAdmin = new IceInternal.MetricsAdminI(initData.properties, Ice.Util.getProcessLogger());
                    initData.observer = new IceInternal.CommunicatorObserverI(metricsAdmin);
                }

                _sharedCommunicator = Ice.Util.initialize(initData);

                //
                // Ensure the metrics admin plugin uses the same property set as the
                // communicator. This is necessary to correctly deal with runtime 
                // property updates.
                //
                if(metricsAdmin != null)
                {
                    metricsAdmin.setProperties(_sharedCommunicator.getProperties());
                }
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
        }
        catch(FailureException ex)
        {
            _logger.error(ex.ToString());
            return 1;
        }
        catch(Exception ex)
        {
            _logger.error("ServiceManager: caught exception:\n" + ex.ToString());
            return 1;
        }
        finally
        {
            //
            // Invoke stop() on the services.
            //
            stopAll();
        }

        return 0;
    }

    private void startService(string service, string entryPoint, string[] args)
    {
        _m.Lock();
        try
        {
            //
            // Extract the assembly name and the class name.
            //
            string err = "ServiceManager: unable to load service '" + entryPoint + "': ";
            int sepPos = entryPoint.IndexOf(':');
            if(sepPos != -1 && IceInternal.AssemblyUtil.platform_ == IceInternal.AssemblyUtil.Platform.Windows)
            {
                const string driveLetters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
                if(entryPoint.Length > 3 &&
                   sepPos == 1 &&
                   driveLetters.IndexOf(entryPoint[0]) != -1 &&
                   (entryPoint[2] == '\\' || entryPoint[2] == '/'))
                {
                    sepPos = entryPoint.IndexOf(':', 3);
                }
            }
            if(sepPos == -1)
            {
                FailureException e = new FailureException();
                e.reason = err + "invalid entry point format";
                throw e;
            }

            System.Reflection.Assembly serviceAssembly = null;
            string assemblyName = entryPoint.Substring(0, sepPos);
            string className = entryPoint.Substring(sepPos + 1);

            try
            {
                //
                // First try to load the assembly using Assembly.Load, which will succeed
                // if a fully-qualified name is provided or if a partial name has been qualified
                // in configuration. If that fails, try Assembly.LoadFrom(), which will succeed
                // if a file name is configured or a partial name is configured and DEVPATH is used.
                //
                try
                {
                    serviceAssembly = System.Reflection.Assembly.Load(assemblyName);
                }
                catch(System.IO.IOException ex)
                {
                    try
                    {
                        serviceAssembly = System.Reflection.Assembly.LoadFrom(assemblyName);
                    }
                    catch(System.IO.IOException)
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
            System.Type c = null;
            try
            {
                c = serviceAssembly.GetType(className, true);
            }
            catch(System.Exception ex)
            {
                FailureException e = new FailureException(ex);
                e.reason = err + "GetType failed for '" + className + "'";
                throw e;
            }

            ServiceInfo info = new ServiceInfo();
            info.name = service;
            info.status = ServiceStatus.Stopped;
            info.args = args;
            
            //
            // If IceBox.UseSharedCommunicator.<name> is defined, create a
            // communicator for the service. The communicator inherits
            // from the shared communicator properties. If it's not
            // defined, add the service properties to the shared
            // commnunicator property set.
            //
            Ice.Communicator communicator;
            IceInternal.MetricsAdminI metricsAdmin = null;
            if(_communicator.getProperties().getPropertyAsInt("IceBox.UseSharedCommunicator." + service) > 0)
            {
                Debug.Assert(_sharedCommunicator != null);
                communicator = _sharedCommunicator;
                if(communicator.getObserver() is IceInternal.CommunicatorObserverI)
                {
                    IceInternal.CommunicatorObserverI o = (IceInternal.CommunicatorObserverI)communicator.getObserver();
                    metricsAdmin = o.getMetricsAdmin();
                }
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
                // Clone the logger to assign a new prefix. If one of the built-in loggers is configured
                // don't set any logger.
                //
                if(initData.properties.getProperty("Ice.LogFile").Length == 0 &&
                   (initData.properties.getPropertyAsInt("Ice.UseSyslog") == 0 ||
                    IceInternal.AssemblyUtil.platform_ == IceInternal.AssemblyUtil.Platform.Windows))
                {
                    initData.logger = _logger.cloneWithPrefix(initData.properties.getProperty("Ice.ProgramName"));
                }

                //
                // If Ice metrics are enabled on the IceBox communicator, we also enable them on
                // the service communicator.
                // 
                if(_communicator.getObserver() is IceInternal.CommunicatorObserverI)
                {
                    metricsAdmin = new IceInternal.MetricsAdminI(initData.properties, initData.logger);
                    initData.observer = new IceInternal.CommunicatorObserverI(metricsAdmin);
                }

                //
                // Remaining command line options are passed to the communicator. This is
                // necessary for Ice plug-in properties (e.g.: IceSSL).
                //
                info.communicator = Ice.Util.initialize(ref info.args, initData);
                communicator = info.communicator;

                //
                // Ensure the metrics admin plugin uses the same property set as the
                // communicator. This is necessary to correctly deal with runtime 
                // property updates.
                //
                if(metricsAdmin != null)
                {
                    metricsAdmin.setProperties(communicator.getProperties());
                }
            }

            try
            {   
                //
                // Add a PropertiesAdmin facet to the service manager's communicator that provides
                // access to this service's property set. We do this prior to instantiating the
                // service so that the service's constructor is able to access the facet (e.g.,
                // in case it wants to set a callback).
                //
                string facetName = "IceBox.Service." + info.name + ".Properties";
                IceInternal.PropertiesAdminI propAdmin = new IceInternal.PropertiesAdminI(facetName, 
                                                                                          communicator.getProperties(), 
                                                                                          communicator.getLogger());
                _communicator.addAdminFacet(propAdmin, facetName);
                
                //
                // If a metrics admin facet is setup for the service, register
                // it with the IceBox communicator.
                //
                if(metricsAdmin != null)
                {
                    _communicator.addAdminFacet(metricsAdmin, "IceBox.Service." + info.name + ".Metrics");

                    // Ensure the metrics admin facet is notified of property updates.
                    propAdmin.addUpdateCallback(metricsAdmin);
                }

                //
                // Instantiate the service.
                //
                try
                {
                    //
                    // If the service class provides a constructor that accepts an Ice.Communicator argument,
                    // use that in preference to the default constructor.
                    //
                    Type[] parameterTypes = new Type[1];
                    parameterTypes[0] = typeof(Ice.Communicator);
                    System.Reflection.ConstructorInfo ci = c.GetConstructor(parameterTypes);
                    if(ci != null)
                    {
                        try
                        {
                            Object[] parameters = new Object[1];
                            parameters[0] = _communicator;
                            info.service = (Service)ci.Invoke(parameters);
                        }
                        catch(System.MethodAccessException ex)
                        {
                            FailureException e = new FailureException(ex);
                            e.reason = err + "unable to access service constructor " + className + "(Ice.Communicator)";
                            throw e;
                        }
                    }
                    else
                    {
                        //
                        // Fall back to the default constructor.
                        //
                        try
                        {
                            info.service = (Service)IceInternal.AssemblyUtil.createInstance(c);
                            if(info.service == null)
                            {
                                FailureException e = new FailureException();
                                e.reason = err + "no default constructor for '" + className + "'";
                                throw e;
                            }
                        }
                        catch(System.UnauthorizedAccessException ex)
                        {
                            FailureException e = new FailureException(ex);
                            e.reason = err + "unauthorized access to default service constructor for " + className;
                            throw e;
                        }
                    }
                }
                catch(FailureException)
                {
                    throw;
                }
                catch(System.InvalidCastException ex)
                {
                    FailureException e = new FailureException(ex);
                    e.reason = err + "service does not implement IceBox.Service";
                    throw e;
                }
                catch(System.Reflection.TargetInvocationException ex)
                {
                    if(ex.InnerException is IceBox.FailureException)
                    {
                        throw ex.InnerException;
                    }
                    else
                    {
                        FailureException e = new FailureException(ex.InnerException);
                        e.reason = err + "exception in service constructor for " + className;
                        throw e;
                    }
                }
                catch(System.Exception ex)
                {
                    FailureException e = new FailureException(ex);
                    e.reason = err + "exception in service constructor " + className;
                    throw e;
                }


                try
                {
                    info.service.start(service, communicator, info.args);
                }
                catch(FailureException)
                {
                    throw;
                }
                catch(System.Exception ex)
                {
                    FailureException e = new FailureException(ex);
                    e.reason = "exception while starting service " + service;
                    throw e;
                }

                info.status = ServiceStatus.Started;
                _services.Add(info);
            }
            catch(Ice.ObjectAdapterDeactivatedException)
            {
                //
                // Can be raised by addAdminFacet if the service manager communicator has been shut down.
                //
                if(info.communicator != null)
                {
                    destroyServiceCommunicator(service, info.communicator);
                }
            }
            catch(System.Exception ex)
            {
                try
                {
                    _communicator.removeAdminFacet("IceBox.Service." + service + ".Properties");
                }
                catch(Ice.LocalException)
                {
                    // Ignored
                }

                if(info.communicator != null)
                {
                    destroyServiceCommunicator(service, info.communicator);
                }

                throw ex;
            }

        }
        finally
        {
            _m.Unlock();
        }
    }

    private void stopAll()
    {
        _m.Lock();
        try
        {
            //
            // First wait for any active startService/stopService calls to complete.
            //
            while(_pendingStatusChanges)
            {
                _m.Wait();
            }

            //
            // For each service, we call stop on the service and flush its database environment to
            // the disk. Services are stopped in the reverse order of the order they were started.
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
                    catch(System.Exception e)
                    {
                        _logger.warning("IceBox.ServiceManager: exception while stopping service " + info.name + ":\n" +
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
                    destroyServiceCommunicator(info.name, info.communicator);
                }
            }

            if(_sharedCommunicator != null)
            {
                try
                {
                    _sharedCommunicator.destroy();
                }
                catch(System.Exception e)
                {
                    _logger.warning("ServiceManager: exception while destroying shared communicator:\n" + e.ToString());
                }
                _sharedCommunicator = null;
            }

            _services.Clear();
            servicesStopped(stoppedServices, _observers.Keys);
        }
        finally
        {
            _m.Unlock();
        }
    }

    private void servicesStarted(List<String> services, Dictionary<ServiceObserverPrx, bool>.KeyCollection observers)
    {
        //
        // Must be called with 'this' unlocked
        //

        if(services.Count > 0)
        {
            string[] servicesArray = services.ToArray();

            foreach(ServiceObserverPrx observer in observers)
            {
                observer.begin_servicesStarted(servicesArray, this.observerCompleted, null);
            }
        }
    }

    private void servicesStopped(List<string> services, Dictionary<ServiceObserverPrx, bool>.KeyCollection observers)
    {
        //
        // Must be called with 'this' unlocked
        //

        if(services.Count > 0)
        {
            string[] servicesArray = services.ToArray();

            foreach(ServiceObserverPrx observer in observers)
            {
                observer.begin_servicesStopped(servicesArray, this.observerCompleted, null);
            }
        }
    }

    private void
    observerCompleted(Ice.AsyncResult result)
    {
        try
        {
            result.throwLocalException();
        }
        catch(Ice.LocalException ex)
        {
            _m.Lock();
            try
            {
                ServiceObserverPrx observer = ServiceObserverPrxHelper.uncheckedCast(result.getProxy());
                if(_observers.Remove(observer))
                {
                    observerRemoved(observer, ex);
                }
            }
            finally
            {
                _m.Unlock();
            }
        }
    }

    private void observerRemoved(ServiceObserverPrx observer, System.Exception ex)
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

            try
            {
                args = IceUtilInternal.Options.split(value);
            }
            catch(IceUtilInternal.Options.BadQuote ex)
            {
                FailureException e = new FailureException();
                e.reason = "ServiceManager: invalid arguments for service `" + name + "':\n" + ex.Message;
                throw e;
            }

            Debug.Assert(args.Length > 0);

            entryPoint = args[0];

            //
            // Shift the arguments.
            //
            string[] tmp = new string[args.Length - 1];
            Array.Copy(args, 1, tmp, 0, args.Length - 1);
            args = tmp;

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

    private Ice.Properties createServiceProperties(String service)
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

    private void destroyServiceCommunicator(string service, Ice.Communicator communicator)
    {
        if(communicator != null)
        {
            try
            {
                communicator.shutdown();
                communicator.waitForShutdown();
            }
            catch(Ice.CommunicatorDestroyedException)
            {
                //
                // Ignore, the service might have already destroyed
                // the communicator for its own reasons.
                //
            }
            catch(System.Exception e)
            {
                _logger.warning("ServiceManager: exception while shutting down communicator for service "
                                + service + "\n" + e.ToString());
            }

            try
            {
                communicator.destroy();
            }
            catch(System.Exception e)
            {
                _logger.warning("ServiceManager: exception while destroying communicator for service "
                                + service + "\n" + e.ToString());
            }
        }
    }

    private Ice.Communicator _communicator;
    private Ice.Communicator _sharedCommunicator = null;
    private Ice.Logger _logger;
    private string[] _argv; // Filtered server argument vector
    private List<ServiceInfo> _services = new List<ServiceInfo>();
    private bool _pendingStatusChanges = false;
    private Dictionary<ServiceObserverPrx, bool> _observers = new  Dictionary<ServiceObserverPrx, bool>();
    private int _traceServiceObserver = 0;
    private readonly IceUtilInternal.Monitor _m = new IceUtilInternal.Monitor();
}

}
