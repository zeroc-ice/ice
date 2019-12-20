//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Linq;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading.Tasks;

using Ice;

namespace IceBox
{
    //
    // NOTE: the class isn't final on purpose to allow users to extend it.
    //
    public class ServiceManagerI : ServiceManager
    {
        public ServiceManagerI(Ice.Communicator communicator, string[] args)
        {
            _communicator = communicator;
            _logger = _communicator.Logger;

            if (_communicator.GetProperty("Ice.Admin.Enabled") == null)
            {
                _adminEnabled = _communicator.GetProperty("Ice.Admin.Endpoints") != null;
            }
            else
            {
                _adminEnabled = _communicator.GetPropertyAsInt("Ice.Admin.Enabled") > 0;
            }

            if (_adminEnabled)
            {
                _adminFacetFilter = new HashSet<string>(
                    _communicator.GetPropertyAsList("Ice.Admin.Facets") ?? Array.Empty<string>());
            }

            _argv = args;
            _traceServiceObserver = _communicator.GetPropertyAsInt("IceBox.Trace.ServiceObserver") ?? 0;
        }

        public void startService(string name, Ice.Current current)
        {
            ServiceInfo info = new ServiceInfo();
            lock (this)
            {
                //
                // Search would be more efficient if services were contained in
                // a map, but order is required for shutdown.
                //
                int i;
                for (i = 0; i < _services.Count; ++i)
                {
                    info = _services[i];
                    if (info.name.Equals(name))
                    {
                        if (_services[i].status != ServiceStatus.Stopped)
                        {
                            throw new AlreadyStartedException();
                        }
                        info.status = ServiceStatus.Starting;
                        _services[i] = info;
                        break;
                    }
                }
                if (i == _services.Count)
                {
                    throw new NoSuchServiceException();
                }
                _pendingStatusChanges = true;
            }

            bool started = false;
            try
            {
                Debug.Assert(info.service != null);
                Debug.Assert(info.name != null);
                Debug.Assert(info.args != null);
                Ice.Communicator? communicator = info.communicator == null ? _sharedCommunicator : info.communicator;
                Debug.Assert(communicator != null);
                info.service.start(info.name, communicator, info.args);
                started = true;
            }
            catch (System.Exception e)
            {
                _logger.warning("ServiceManager: exception while starting service " + info.name + ":\n" + e.ToString());
            }

            lock (this)
            {
                int i;
                for (i = 0; i < _services.Count; ++i)
                {
                    info = _services[i];
                    if (info.name.Equals(name))
                    {
                        if (started)
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
                System.Threading.Monitor.PulseAll(this);
            }
        }

        public void stopService(string name, Ice.Current current)
        {
            ServiceInfo info = new ServiceInfo();
            lock (this)
            {
                //
                // Search would be more efficient if services were contained in
                // a map, but order is required for shutdown.
                //
                int i;
                for (i = 0; i < _services.Count; ++i)
                {
                    info = _services[i];
                    if (info.name.Equals(name))
                    {
                        if (info.status != ServiceStatus.Started)
                        {
                            throw new AlreadyStoppedException();
                        }
                        info.status = ServiceStatus.Stopping;
                        _services[i] = info;
                        break;
                    }
                }
                if (i == _services.Count)
                {
                    throw new NoSuchServiceException();
                }
                _pendingStatusChanges = true;
            }

            bool stopped = false;
            try
            {
                Debug.Assert(info.service != null);
                info.service.stop();
                stopped = true;
            }
            catch (System.Exception e)
            {
                _logger.warning("ServiceManager: exception while stopping service " + info.name + "\n" + e.ToString());
            }

            lock (this)
            {
                int i;
                for (i = 0; i < _services.Count; ++i)
                {
                    info = _services[i];
                    if (info.name.Equals(name))
                    {
                        if (stopped)
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
                System.Threading.Monitor.PulseAll(this);
            }
        }

        public void addObserver(ServiceObserverPrx observer, Ice.Current current)
        {
            List<string> activeServices = new List<string>();

            //
            // Null observers and duplicate registrations are ignored
            //
            lock (this)
            {
                if (observer != null)
                {
                    try
                    {
                        _observers.Add(observer, true);
                    }
                    catch (ArgumentException)
                    {
                        return;
                    }

                    if (_traceServiceObserver >= 1)
                    {
                        _logger.trace("IceBox.ServiceObserver", $"Added service observer {observer}");
                    }

                    foreach (ServiceInfo info in _services)
                    {
                        if (info.status == ServiceStatus.Started)
                        {
                            activeServices.Add(info.name);
                        }
                    }
                }
            }

            if (activeServices.Count > 0)
            {
                observer!.servicesStartedAsync(activeServices.ToArray()).ContinueWith((t) => observerCompleted(observer, t),
                    TaskScheduler.Current);
            }
        }

        public void shutdown(Ice.Current current)
        {
            _communicator.shutdown();
        }

        public int run()
        {
            try
            {
                //
                // Create an object adapter. Services probably should NOT share
                // this object adapter, as the endpoint(s) for this object adapter
                // will most likely need to be firewalled for security reasons.
                //
                Ice.ObjectAdapter? adapter = null;
                if (_communicator.GetProperty("IceBox.ServiceManager.Endpoints") != null)
                {
                    adapter = _communicator.createObjectAdapter("IceBox.ServiceManager");
                    adapter.Add(this, new Ice.Identity("ServiceManager",
                        _communicator.GetProperty("IceBox.InstanceName") ?? "IceBox"));
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
                Dictionary<string, string> services = _communicator.GetProperties(forPrefix: prefix);

                if (services.Count == 0)
                {
                    throw new FailureException("ServiceManager: configuration must include at least one IceBox service");
                }

                string[] loadOrder = (_communicator.GetPropertyAsList("IceBox.LoadOrder") ?? Array.Empty<string>()).Where(
                    s => s.Length > 0).ToArray();
                List<StartServiceInfo> servicesInfo = new List<StartServiceInfo>();
                foreach (var o in loadOrder)
                {
                    string key = prefix + o;
                    string? value;
                    if (!services.TryGetValue(key, out value))
                    {
                        throw new FailureException($"ServiceManager: no service definition for `{o}'");
                    }
                    servicesInfo.Add(new StartServiceInfo(o, value, _argv));
                    services.Remove(key);
                }

                foreach (KeyValuePair<string, string> entry in services)
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
                if (_communicator.GetProperties(forPrefix: "IceBox.UseSharedCommunicator.").Count > 0)
                {
                    Dictionary<string, string> properties = CreateServiceProperties("SharedCommunicator");
                    foreach (StartServiceInfo service in servicesInfo)
                    {
                        if ((_communicator.GetPropertyAsInt($"IceBox.UseSharedCommunicator.{service.name}") ?? 0) <= 0)
                        {
                            continue;
                        }

                        //
                        // Load the service properties using the shared communicator properties as the default properties.
                        //
                        properties.ParseIceArgs(ref service.args);

                        //
                        // Parse <service>.* command line options (the Ice command line options
                        // were parsed by the call to createProperties above).
                        //
                        properties.ParseArgs(ref service.args, service.name);
                    }

                    string facetNamePrefix = "IceBox.SharedCommunicator.";
                    bool addFacets = ConfigureAdmin(properties, facetNamePrefix);

                    _sharedCommunicator = new Ice.Communicator(properties);

                    if (addFacets)
                    {
                        // Add all facets created on shared communicator to the IceBox communicator
                        // but renamed <prefix>.<facet-name>, except for the Process facet which is
                        // never added.
                        foreach (var p in _sharedCommunicator.FindAllAdminFacets())
                        {
                            if (!p.Key.Equals("Process"))
                            {
                                _communicator.AddAdminFacet(p.Value.servant, p.Value.disp, facetNamePrefix + p.Key);
                            }
                        }
                    }
                }

                foreach (StartServiceInfo s in servicesInfo)
                {
                    startService(s.name, s.entryPoint, s.args);
                }

                //
                // Start Admin (if enabled) and/or deprecated IceBox.ServiceManager OA
                //
                _communicator.AddAdminFacet<ServiceManager, ServiceManagerTraits>(this, "IceBox.ServiceManager");
                _communicator.getAdmin();
                if (adapter != null)
                {
                    adapter.Activate();
                }

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
                string? bundleName = _communicator.GetProperty("IceBox.PrintServicesReady");
                if (bundleName != null)
                {
                    Console.Out.WriteLine(bundleName + " ready");
                }

                _communicator.waitForShutdown();
            }
            catch (FailureException ex)
            {
                _logger.error(ex.ToString());
                return 1;
            }
            catch (Ice.CommunicatorDestroyedException)
            {
                // Expected if the communicator is shutdown
            }
            catch (Ice.ObjectAdapterDeactivatedException)
            {
                // Expected if the mmunicator is shutdown
            }
            catch (System.Exception ex)
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
            lock (this)
            {
                //
                // Extract the assembly name and the class name.
                //
                string err = "ServiceManager: unable to load service '" + entryPoint + "': ";
                int sepPos = entryPoint.IndexOf(':');
                if (sepPos != -1)
                {
                    const string driveLetters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
                    if (entryPoint.Length > 3 &&
                       sepPos == 1 &&
                       driveLetters.IndexOf(entryPoint[0]) != -1 &&
                       (entryPoint[2] == '\\' || entryPoint[2] == '/'))
                    {
                        sepPos = entryPoint.IndexOf(':', 3);
                    }
                }
                if (sepPos == -1)
                {
                    FailureException e = new FailureException();
                    e.reason = err + "invalid entry point format";
                    throw e;
                }

                System.Reflection.Assembly? serviceAssembly = null;
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
                    catch (System.Exception ex)
                    {
                        try
                        {
                            serviceAssembly = System.Reflection.Assembly.LoadFrom(assemblyName);
                        }
                        catch (System.Exception)
                        {
#pragma warning disable CA2200 // Rethrow to preserve stack details.
                            throw ex;
#pragma warning restore CA2200 // Rethrow to preserve stack details.
                        }
                    }
                }
                catch (System.Exception ex)
                {
                    FailureException e = new FailureException(ex);
                    e.reason = err + "unable to load assembly: " + assemblyName;
                    throw e;
                }

                //
                // Instantiate the class.
                //
                Type? c = null;
                try
                {
                    c = serviceAssembly.GetType(className, true);
                }
                catch (System.Exception ex)
                {
                    FailureException e = new FailureException(ex);
                    e.reason = err + "GetType failed for '" + className + "'";
                    throw e;
                }

                ServiceInfo info = new ServiceInfo();
                info.name = service;
                info.status = ServiceStatus.Stopped;
                info.args = args;

                Logger? logger = null;
                //
                // If IceBox.UseSharedCommunicator.<name> is defined, create a
                // communicator for the service. The communicator inherits
                // from the shared communicator properties. If it's not
                // defined, add the service properties to the shared
                // commnunicator property set.
                //
                Communicator communicator;
                if (_communicator.GetPropertyAsInt($"IceBox.UseSharedCommunicator.{service}") > 0)
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
                    Dictionary<string, string> properties = CreateServiceProperties(service);
                    if (info.args.Length > 0)
                    {
                        //
                        // Create the service properties with the given service arguments. This should
                        // read the service config file if it's specified with --Ice.Config.
                        //
                        properties.ParseIceArgs(ref info.args);

                        //
                        // Next, parse the service "<service>.*" command line options (the Ice command
                        // line options were parsed by the createProperties above)
                        //
                        properties.ParseArgs(ref info.args, service);
                    }

                    //
                    // Clone the logger to assign a new prefix. If one of the built-in loggers is configured
                    // don't set any logger.
                    //
                    string? logFile;
                    if (properties.TryGetValue("Ice.LogFile", out logFile))
                    {
                        logger = _logger.cloneWithPrefix(properties.GetValueOrDefault("Ice.ProgramName") ?? "");
                    }

                    //
                    // If Admin is enabled on the IceBox communicator, for each service that does not set
                    // Ice.Admin.Enabled, we set Ice.Admin.Enabled=1 to have this service create facets; then
                    // we add these facets to the IceBox Admin object as IceBox.Service.<service>.<facet>.
                    //
                    string serviceFacetNamePrefix = "IceBox.Service." + service + ".";
                    bool addFacets = ConfigureAdmin(properties, serviceFacetNamePrefix);

                    //
                    // Remaining command line options are passed to the communicator. This is
                    // necessary for Ice plug-in properties (e.g.: IceSSL).
                    //
                    info.communicator = new Ice.Communicator(ref info.args, properties, logger: logger);
                    communicator = info.communicator;

                    if (addFacets)
                    {
                        // Add all facets created on the service communicator to the IceBox communicator
                        // but renamed IceBox.Service.<service>.<facet-name>, except for the Process facet
                        // which is never added
                        foreach (var p in communicator.FindAllAdminFacets())
                        {
                            if (!p.Key.Equals("Process"))
                            {
                                _communicator.AddAdminFacet(p.Value.servant, p.Value.disp, serviceFacetNamePrefix + p.Key);
                            }
                        }
                    }
                }

                try
                {
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
                        System.Reflection.ConstructorInfo? ci = c.GetConstructor(parameterTypes);
                        if (ci != null)
                        {
                            try
                            {
                                object[] parameters = new object[1];
                                parameters[0] = _communicator;
                                info.service = (Service)ci.Invoke(parameters);
                            }
                            catch (MethodAccessException ex)
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
                                Service? s = (Service?)IceInternal.AssemblyUtil.createInstance(c);
                                if (s == null)
                                {
                                    FailureException e = new FailureException();
                                    e.reason = err + "no default constructor for '" + className + "'";
                                    throw e;
                                }
                                info.service = s;
                            }
                            catch (UnauthorizedAccessException ex)
                            {
                                FailureException e = new FailureException(ex);
                                e.reason = err + "unauthorized access to default service constructor for " + className;
                                throw e;
                            }
                        }
                    }
                    catch (FailureException)
                    {
                        throw;
                    }
                    catch (InvalidCastException ex)
                    {
                        FailureException e = new FailureException(ex);
                        e.reason = err + "service does not implement IceBox.Service";
                        throw e;
                    }
                    catch (System.Reflection.TargetInvocationException ex)
                    {
                        if (ex.InnerException is FailureException)
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
                    catch (System.Exception ex)
                    {
                        throw new FailureException($"{err} exception in service constructor {className}", ex);
                    }

                    try
                    {
                        info.service.start(service, communicator, info.args);
                    }
                    catch (FailureException)
                    {
                        throw;
                    }
                    catch (System.Exception ex)
                    {
                        throw new FailureException($"exception while starting service {service}", ex);
                    }

                    info.status = ServiceStatus.Started;
                    _services.Add(info);
                }
                catch (System.Exception)
                {
                    if (info.communicator != null)
                    {
                        destroyServiceCommunicator(service, info.communicator);
                    }

                    throw;
                }

            }
        }

        private void stopAll()
        {
            lock (this)
            {
                //
                // First wait for any active startService/stopService calls to complete.
                //
                while (_pendingStatusChanges)
                {
                    System.Threading.Monitor.Wait(this);
                }

                //
                // For each service, we call stop on the service and flush its database environment to
                // the disk. Services are stopped in the reverse order of the order they were started.
                //
                _services.Reverse();
                List<string> stoppedServices = new List<string>();
                foreach (ServiceInfo info in _services)
                {
                    if (info.status == ServiceStatus.Started)
                    {
                        try
                        {
                            info.service.stop();
                            stoppedServices.Add(info.name);
                        }
                        catch (System.Exception e)
                        {
                            _logger.warning("IceBox.ServiceManager: exception while stopping service " + info.name + ":\n" +
                                            e.ToString());
                        }
                    }

                    if (info.communicator != null)
                    {
                        destroyServiceCommunicator(info.name, info.communicator);
                    }
                }

                if (_sharedCommunicator != null)
                {
                    removeAdminFacets("IceBox.SharedCommunicator.");

                    try
                    {
                        _sharedCommunicator.destroy();
                    }
                    catch (System.Exception e)
                    {
                        _logger.warning("ServiceManager: exception while destroying shared communicator:\n" + e.ToString());
                    }
                    _sharedCommunicator = null;
                }

                _services.Clear();
                servicesStopped(stoppedServices, _observers.Keys);
            }
        }

        private void servicesStarted(List<string> services, Dictionary<ServiceObserverPrx, bool>.KeyCollection observers)
        {
            //
            // Must be called with 'this' unlocked
            //

            if (services.Count > 0)
            {
                string[] servicesArray = services.ToArray();

                foreach (ServiceObserverPrx observer in observers)
                {
                    observer.servicesStartedAsync(servicesArray).ContinueWith((t) => observerCompleted(observer, t),
                        TaskScheduler.Current);
                }
            }
        }

        private void servicesStopped(List<string> services, Dictionary<ServiceObserverPrx, bool>.KeyCollection observers)
        {
            //
            // Must be called with 'this' unlocked
            //

            if (services.Count > 0)
            {
                string[] servicesArray = services.ToArray();

                foreach (ServiceObserverPrx observer in observers)
                {
                    observer.servicesStoppedAsync(servicesArray).ContinueWith((t) => observerCompleted(observer, t),
                        TaskScheduler.Current);
                }
            }
        }

        private void
        observerCompleted(ServiceObserverPrx observer, Task t)
        {
            try
            {
                t.Wait();
            }
            catch (AggregateException ae)
            {
                lock (this)
                {
                    if (_observers.Remove(observer))
                    {
                        observerRemoved(observer, ae.InnerException);
                    }
                }
            }
        }

        private void observerRemoved(ServiceObserverPrx observer, System.Exception ex)
        {
            if (_traceServiceObserver >= 1)
            {
                //
                // CommunicatorDestroyedException may occur during shutdown. The observer notification has
                // been sent, but the communicator was destroyed before the reply was received. We do not
                // log a message for this exception.
                //
                if (!(ex is Ice.CommunicatorDestroyedException))
                {
                    _logger.trace("IceBox.ServiceObserver",
                                  $"Removed service observer {observer}\nafter catching {ex}");
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

        private struct ServiceInfo
        {
            public string name;
            public Service service;
            public Ice.Communicator communicator;
            public ServiceStatus status;
            public string[] args;
        }

        private class StartServiceInfo
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
                catch (FormatException ex)
                {
                    FailureException e = new FailureException();
                    e.reason = "ServiceManager: invalid arguments for service `" + name + "':\n" + ex.Message;
                    throw e;
                }

                Debug.Assert(args.Length > 0);

                entryPoint = args[0];
                args = args.Skip(1).Concat(serverArgs.Where(arg => arg.StartsWith($"--{service}."))).ToArray();
            }

            public string name;
            public string entryPoint;
            public string[] args;
        }

        private Dictionary<string, string> CreateServiceProperties(string service)
        {
            Dictionary<string, string> properties;
            if ((_communicator.GetPropertyAsInt("IceBox.InheritProperties") ?? 0) > 0)
            {
                // Inherit all except Ice.Admin.xxx properties
                properties = _communicator.GetProperties().Where(p => !p.Key.StartsWith("Ice.Admin.")).ToDictionary(p => p.Key, p => p.Value);
            }
            else
            {
                properties = new Dictionary<string, string>();
            }

            string? programName = _communicator.GetProperty("Ice.ProgramName");
            properties["Ice.ProgramName"] = programName == null ? service : $"{programName}-{service}";
            return properties;
        }

        private void destroyServiceCommunicator(string service, Ice.Communicator communicator)
        {
            if (communicator != null)
            {
                try
                {
                    communicator.shutdown();
                    communicator.waitForShutdown();
                }
                catch (Ice.CommunicatorDestroyedException)
                {
                    //
                    // Ignore, the service might have already destroyed
                    // the communicator for its own reasons.
                    //
                }
                catch (System.Exception e)
                {
                    _logger.warning("ServiceManager: exception while shutting down communicator for service "
                                    + service + "\n" + e.ToString());
                }

                removeAdminFacets("IceBox.Service." + service + ".");
                communicator.destroy();
            }
        }

        private bool ConfigureAdmin(Dictionary<string, string> properties, string prefix)
        {
            if (_adminEnabled && !properties.ContainsKey("Ice.Admin.Enabled"))
            {
                List<string> facetNames = new List<string>();
                Debug.Assert(_adminFacetFilter != null);
                foreach (string p in _adminFacetFilter)
                {
                    if (p.StartsWith(prefix))
                    {
                        facetNames.Add(p.Substring(prefix.Length));
                    }
                }

                if (_adminFacetFilter.Count == 0 || facetNames.Count > 0)
                {
                    properties["Ice.Admin.Enabled"] = "1";

                    if (facetNames.Count > 0)
                    {
                        // TODO: need String.Join with escape!
                        properties["Ice.Admin.Facets"] = string.Join(" ", facetNames.ToArray());
                    }
                    return true;
                }
            }
            return false;
        }

        private void removeAdminFacets(string prefix)
        {
            try
            {
                foreach (string p in _communicator.FindAllAdminFacets().Keys)
                {
                    if (p.StartsWith(prefix))
                    {
                        _communicator.RemoveAdminFacet(p);
                    }
                }
            }
            catch (Ice.CommunicatorDestroyedException)
            {
                // Ignored
            }
            catch (Ice.ObjectAdapterDeactivatedException)
            {
                // Ignored
            }
        }

        private Ice.Communicator _communicator;
        private bool _adminEnabled = false;
        private HashSet<string>? _adminFacetFilter = null;
        private Ice.Communicator? _sharedCommunicator = null;
        private Ice.Logger _logger;
        private string[] _argv; // Filtered server argument vector
        private List<ServiceInfo> _services = new List<ServiceInfo>();
        private bool _pendingStatusChanges = false;
        private Dictionary<ServiceObserverPrx, bool> _observers = new Dictionary<ServiceObserverPrx, bool>();
        private int _traceServiceObserver = 0;
    }

}
