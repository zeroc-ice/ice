//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Linq;
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
        public ServiceManagerI(Communicator communicator, string[] args)
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

        public void startService(string name, Current current)
        {
            ServiceInfo? info;
            lock (this)
            {
                //
                // Search would be more efficient if services were contained in
                // a map, but order is required for shutdown.
                //
                info = _services.Find(service => service.Name.Equals(name));
                if (info == null)
                {
                    throw new NoSuchServiceException();
                }

                if (info.Status != ServiceStatus.Stopped)
                {
                    throw new AlreadyStartedException();
                }
                info.Status = ServiceStatus.Starting;
                _pendingStatusChanges = true;
            }

            bool started = false;
            try
            {
                Debug.Assert(info.Service != null);
                Communicator? communicator = info.Communicator == null ? _sharedCommunicator : info.Communicator;
                Debug.Assert(communicator != null);
                info.Service.start(info.Name, communicator, info.Args);
                started = true;
            }
            catch (System.Exception ex)
            {
                _logger.warning($"ServiceManager: exception while starting service {info.Name}:\n{ex}");
            }

            lock (this)
            {
                if (started)
                {
                    info.Status = ServiceStatus.Started;
                    servicesStarted(new List<string>() { name }, _observers.Keys);
                }
                else
                {
                    info.Status = ServiceStatus.Stopped;
                }
                _pendingStatusChanges = false;
                System.Threading.Monitor.PulseAll(this);
            }
        }

        public void stopService(string name, Current current)
        {
            ServiceInfo? info;
            lock (this)
            {
                //
                // Search would be more efficient if services were contained in
                // a map, but order is required for shutdown.
                //
                info = _services.Find(service => service.Name.Equals(name));
                if (info == null)
                {
                    throw new NoSuchServiceException();
                }

                if (info.Status != ServiceStatus.Started)
                {
                    throw new AlreadyStoppedException();
                }
                _pendingStatusChanges = true;
            }

            bool stopped = false;
            try
            {
                Debug.Assert(info.Service != null);
                info.Service.stop();
                stopped = true;
            }
            catch (System.Exception ex)
            {
                _logger.warning($"ServiceManager: exception while stopping service {info.Name}\n{ex}");
            }

            lock (this)
            {
                if (stopped)
                {
                    info.Status = ServiceStatus.Stopped;
                    servicesStopped(new List<string>() { name }, _observers.Keys);
                }
                else
                {
                    info.Status = ServiceStatus.Started;
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
                        if (info.Status == ServiceStatus.Started)
                        {
                            activeServices.Add(info.Name);
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
                ObjectAdapter? adapter = null;
                if (_communicator.GetProperty("IceBox.ServiceManager.Endpoints") != null)
                {
                    adapter = _communicator.createObjectAdapter("IceBox.ServiceManager");
                    adapter.Add(this, new Identity("ServiceManager",
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
                    throw new InvalidOperationException("ServiceManager: configuration must include at least one IceBox service");
                }

                string[] loadOrder = (_communicator.GetPropertyAsList("IceBox.LoadOrder") ?? Array.Empty<string>()).Where(
                    s => s.Length > 0).ToArray();
                List<StartServiceInfo> servicesInfo = new List<StartServiceInfo>();
                foreach (var name in loadOrder)
                {
                    string key = prefix + name;
                    string? value;
                    if (!services.TryGetValue(key, out value))
                    {
                        throw new InvalidOperationException($"ServiceManager: no service definition for `{name}'");
                    }
                    servicesInfo.Add(new StartServiceInfo(name, value, _argv));
                    services.Remove(key);
                }

                foreach (var entry in services)
                {
                    servicesInfo.Add(new StartServiceInfo(entry.Key.Substring(prefix.Length), entry.Value, _argv));
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
                        if ((_communicator.GetPropertyAsInt($"IceBox.UseSharedCommunicator.{service.Name}") ?? 0) <= 0)
                        {
                            continue;
                        }

                        //
                        // Load the service properties using the shared communicator properties as the default properties.
                        //
                        properties.ParseIceArgs(ref service.Args);

                        //
                        // Parse <service>.* command line options (the Ice command line options
                        // were parsed by the call to createProperties above).
                        //
                        properties.ParseArgs(ref service.Args, service.Name);
                    }

                    string facetNamePrefix = "IceBox.SharedCommunicator.";
                    bool addFacets = ConfigureAdmin(properties, facetNamePrefix);

                    _sharedCommunicator = new Communicator(properties);

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
                    startService(s.Name, s.EntryPoint, s.Args);
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
            catch (CommunicatorDestroyedException)
            {
                // Expected if the communicator is shutdown
            }
            catch (ObjectAdapterDeactivatedException)
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
                    throw new FormatException($"invalid entry point format `{entryPoint}");
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
                    throw new InvalidOperationException(
                        $"ServiceManager: unable to load service '{entryPoint}': error loading assembly: {assemblyName}", ex);
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
                    throw new InvalidOperationException(
                        $"ServiceManager: unable to load service '{entryPoint}': cannot find the service class `{className}'", ex);
                }

                ServiceInfo info = new ServiceInfo(service, ServiceStatus.Stopped, args);

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
                    if (info.Args.Length > 0)
                    {
                        //
                        // Create the service properties with the given service arguments. This should
                        // read the service config file if it's specified with --Ice.Config.
                        //
                        properties.ParseIceArgs(ref info.Args);

                        //
                        // Next, parse the service "<service>.*" command line options (the Ice command
                        // line options were parsed by the createProperties above)
                        //
                        properties.ParseArgs(ref info.Args, service);
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
                    info.Communicator = new Communicator(ref info.Args, properties, logger: logger);
                    communicator = info.Communicator;

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
                    Service? s;
                    try
                    {
                        //
                        // If the service class provides a constructor that accepts an Ice.Communicator argument,
                        // use that in preference to the default constructor.
                        //
                        Type[] parameterTypes = new Type[1];
                        parameterTypes[0] = typeof(Communicator);
                        System.Reflection.ConstructorInfo? ci = c.GetConstructor(parameterTypes);
                        if (ci != null)
                        {
                            object[] parameters = new object[1];
                            parameters[0] = _communicator;
                            s = (Service)ci.Invoke(parameters);
                        }
                        else
                        {
                            //
                            // Fall back to the default constructor.
                            //
                            s = (Service?)IceInternal.AssemblyUtil.createInstance(c);
                        }
                    }
                    catch (System.Exception ex)
                    {
                        throw new InvalidOperationException($"ServiceManager: unable to load service '{entryPoint}", ex);
                    }

                    if (s == null)
                    {
                        throw new InvalidOperationException(
                            $"ServiceManager: unable to load service '{entryPoint}': " +
                            $"no default constructor for `{className}'");
                    }
                    info.Service = s;

                    try
                    {
                        info.Service.start(service, communicator, info.Args);
                    }
                    catch (System.Exception ex)
                    {
                        throw new InvalidOperationException($"ServiceManager: exception while starting service {service}", ex);
                    }

                    info.Status = ServiceStatus.Started;
                    _services.Add(info);
                }
                catch (System.Exception)
                {
                    if (info.Communicator != null)
                    {
                        destroyServiceCommunicator(service, info.Communicator);
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
                    if (info.Status == ServiceStatus.Started)
                    {
                        try
                        {
                            Debug.Assert(info.Service != null);
                            info.Service.stop();
                            stoppedServices.Add(info.Name);
                        }
                        catch (System.Exception ex)
                        {
                            _logger.warning($"IceBox.ServiceManager: exception while stopping service {info.Name}:\n{ex}");
                        }
                    }

                    if (info.Communicator != null)
                    {
                        destroyServiceCommunicator(info.Name, info.Communicator);
                    }
                }

                if (_sharedCommunicator != null)
                {
                    removeAdminFacets("IceBox.SharedCommunicator.");

                    try
                    {
                        _sharedCommunicator.destroy();
                    }
                    catch (System.Exception ex)
                    {
                        _logger.warning($"ServiceManager: exception while destroying shared communicator:\n{ex}");
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
                if (!(ex is CommunicatorDestroyedException))
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

        private class ServiceInfo
        {
            internal ServiceInfo(string name, ServiceStatus status, string[] args)
            {
                Name = name;
                Status = status;
                Args = args;
            }
            internal readonly string Name;
            internal ServiceStatus Status;
            internal string[] Args;
            internal Service? Service;
            internal Communicator? Communicator;
        }

        private class StartServiceInfo
        {
            internal StartServiceInfo(string service, string value, string[] serverArgs)
            {
                //
                // Separate the entry point from the arguments.
                //
                Name = service;

                try
                {
                    Args = IceUtilInternal.Options.split(value);
                }
                catch (FormatException ex)
                {
                    throw new ArgumentException($"ServiceManager: invalid arguments for service `{Name}'", ex);
                }

                Debug.Assert(Args.Length > 0);

                EntryPoint = Args[0];
                Args = Args.Skip(1).Concat(serverArgs.Where(arg => arg.StartsWith($"--{service}."))).ToArray();
            }

            internal string Name;
            internal string EntryPoint;
            internal string[] Args;
        }

        private Dictionary<string, string> CreateServiceProperties(string service)
        {
            Dictionary<string, string> properties;
            if ((_communicator.GetPropertyAsInt("IceBox.InheritProperties") ?? 0) > 0)
            {
                // Inherit all except Ice.Admin.xxx properties
                properties = _communicator.GetProperties().Where(p => !p.Key.StartsWith("Ice.Admin.")).ToDictionary(
                    p => p.Key, p => p.Value);
            }
            else
            {
                properties = new Dictionary<string, string>();
            }

            string? programName = _communicator.GetProperty("Ice.ProgramName");
            properties["Ice.ProgramName"] = programName == null ? service : $"{programName}-{service}";
            return properties;
        }

        private void destroyServiceCommunicator(string service, Communicator communicator)
        {
            if (communicator != null)
            {
                try
                {
                    communicator.shutdown();
                    communicator.waitForShutdown();
                }
                catch (CommunicatorDestroyedException)
                {
                    //
                    // Ignore, the service might have already destroyed
                    // the communicator for its own reasons.
                    //
                }
                catch (System.Exception ex)
                {
                    _logger.warning($"ServiceManager: exception while shutting down communicator for service {service}\n{ex}");
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
            catch (CommunicatorDestroyedException)
            {
                // Ignored
            }
            catch (ObjectAdapterDeactivatedException)
            {
                // Ignored
            }
        }

        private Communicator _communicator;
        private bool _adminEnabled = false;
        private HashSet<string>? _adminFacetFilter = null;
        private Communicator? _sharedCommunicator = null;
        private Logger _logger;
        private string[] _argv; // Filtered server argument vector
        private List<ServiceInfo> _services = new List<ServiceInfo>();
        private bool _pendingStatusChanges = false;
        private Dictionary<ServiceObserverPrx, bool> _observers = new Dictionary<ServiceObserverPrx, bool>();
        private int _traceServiceObserver = 0;
    }

}
