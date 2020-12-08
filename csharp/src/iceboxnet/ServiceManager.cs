// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Reflection;
using System.Threading;
using System.Threading.Tasks;

using ZeroC.Ice;

namespace ZeroC.IceBox
{
    internal sealed class ServiceManager : IServiceManager
    {
        private readonly bool _adminEnabled;
        private readonly HashSet<string>? _adminFacetFilter;
        private readonly string[] _argv; // Filtered server argument vector
        private readonly Communicator _communicator;
        private readonly ILogger _logger;
        private readonly object _mutex = new object();
        private readonly HashSet<IServiceObserverPrx> _observers = new HashSet<IServiceObserverPrx>();
        private bool _pendingStatusChanges;
        private readonly List<ServiceInfo> _services = new List<ServiceInfo>();
        private Communicator? _sharedCommunicator;
        private readonly int _traceServiceObserver;

        internal ServiceManager(Communicator communicator, string[] args)
        {
            _communicator = communicator;
            _logger = _communicator.Logger;

            _adminEnabled = _communicator.GetPropertyAsBool("Ice.Admin.Enabled") ??
                _communicator.GetProperty("Ice.Admin.Endpoints") != null;

            if (_adminEnabled)
            {
                _adminFacetFilter = new HashSet<string>(_communicator.GetPropertyAsList("Ice.Admin.Facets") ??
                                                        Array.Empty<string>());
            }

            _argv = args;
            _traceServiceObserver = _communicator.GetPropertyAsInt("IceBox.Trace.ServiceObserver") ?? 0;
        }

        public void AddObserver(IServiceObserverPrx? observer, Current current, CancellationToken cancel)
        {
            // Null observers and duplicate registrations are ignored
            if (observer != null)
            {
                string[] activeServices;
                lock (_mutex)
                {
                    try
                    {
                        _observers.Add(observer);
                    }
                    catch (ArgumentException)
                    {
                        return;
                    }

                    if (_traceServiceObserver >= 1)
                    {
                        _logger.Trace("IceBox.ServiceObserver", $"Added service observer {observer}");
                    }

                    activeServices = _services.Where(info => info.Status == ServiceStatus.Started).Select(
                        info => info.Name).ToArray();
                }

                if (activeServices.Length > 0)
                {
                    _ = StartedAsync(observer, activeServices);
                }

                async Task StartedAsync(IServiceObserverPrx observer, string[] services)
                {
                    try
                    {
                        await observer.ServicesStartedAsync(services, cancel: cancel).ConfigureAwait(false);
                    }
                    catch (Exception ex)
                    {
                        RemoveObserver(observer, ex);
                    }
                }
            }
        }

        public void Shutdown(Current current, CancellationToken cancel) => _ = _communicator.ShutdownAsync();

        public void StartService(string name, Current current, CancellationToken cancel)
        {
            ServiceInfo? info;
            lock (_mutex)
            {
                // Search would be more efficient if services were contained in a map, but order is required for
                // shutdown.
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
                Communicator? communicator = info.Communicator ?? _sharedCommunicator;
                Debug.Assert(communicator != null);
                info.Service.Start(info.Name, communicator, info.Args);
                started = true;
            }
            catch (Exception ex)
            {
                _logger.Warning($"IceBox.ServiceManager: exception while starting service `{info.Name}':\n{ex}");
            }

            lock (_mutex)
            {
                if (started)
                {
                    info.Status = ServiceStatus.Started;
                    ServiceStarted(name, _observers);
                }
                else
                {
                    info.Status = ServiceStatus.Stopped;
                }
                _pendingStatusChanges = false;
                Monitor.PulseAll(_mutex);
            }
        }

        public void StopService(string name, Current current, CancellationToken cancel)
        {
            ServiceInfo? info;
            lock (_mutex)
            {
                // Search would be more efficient if services were contained in a map, but order is required for
                // shutdown.
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
                info.Service.Stop();
                stopped = true;
            }
            catch (Exception ex)
            {
                _logger.Warning($"IceBox.ServiceManager: exception while stopping service `{info.Name}'\n{ex}");
            }

            lock (_mutex)
            {
                if (stopped)
                {
                    info.Status = ServiceStatus.Stopped;
                    ServicesStopped(new string[] { name }, _observers);
                }
                else
                {
                    info.Status = ServiceStatus.Started;
                }
                _pendingStatusChanges = false;
                Monitor.PulseAll(_mutex);
            }
        }

        internal async Task<int> RunAsync()
        {
            try
            {
                // Parse the property set with the prefix "IceBox.Service.". These properties should have the following
                // format:
                //
                // IceBox.Service.Foo=<assembly>:Package.Foo [args]
                //
                // We parse the service properties specified in IceBox.LoadOrder first, then the ones from remaining
                // services.
                string prefix = "IceBox.Service.";
                Dictionary<string, string> services = _communicator.GetProperties(forPrefix: prefix);

                if (services.Count == 0)
                {
                    throw new InvalidConfigurationException(
                        "IceBox.ServiceManager: configuration must include at least one IceBox service");
                }

                string[] loadOrder = (_communicator.GetPropertyAsList("IceBox.LoadOrder") ?? Array.Empty<string>()).Where(
                    s => s.Length > 0).ToArray();
                var servicesInfo = new List<StartServiceInfo>();
                foreach (string name in loadOrder)
                {
                    string key = prefix + name;
                    if (!services.TryGetValue(key, out string? value))
                    {
                        throw new InvalidConfigurationException(
                            $"IceBox.ServiceManager: no service definition for `{name}'");
                    }
                    servicesInfo.Add(new StartServiceInfo(name, value, _argv));
                    services.Remove(key);
                }

                foreach (KeyValuePair<string, string> entry in services)
                {
                    servicesInfo.Add(new StartServiceInfo(entry.Key.Substring(prefix.Length), entry.Value, _argv));
                }

                // Check if some services are using the shared communicator in which case we create the shared
                // communicator now with a property set that is the union of all the service properties (from services
                // that use the shared communicator).
                if (_communicator.GetProperties(forPrefix: "IceBox.UseSharedCommunicator.").Count > 0)
                {
                    Dictionary<string, string> properties = CreateServiceProperties("SharedCommunicator");
                    foreach (StartServiceInfo service in servicesInfo)
                    {
                        if (!(_communicator.GetPropertyAsBool($"IceBox.UseSharedCommunicator.{service.Name}") ?? false))
                        {
                            continue;
                        }

                        // Load the service properties using the shared communicator properties as the default properties.
                        properties.ParseIceArgs(ref service.Args);

                        // Parse <service>.* command line options (the Ice command line options were parsed by the call
                        // to createProperties above).
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
                        foreach (KeyValuePair<string, IObject> p in _sharedCommunicator.FindAllAdminFacets())
                        {
                            if (!p.Key.Equals("Process"))
                            {
                                _communicator.AddAdminFacet(facetNamePrefix + p.Key, p.Value);
                            }
                        }
                    }
                }

                foreach (StartServiceInfo s in servicesInfo)
                {
                    StartService(s.Name, s.EntryPoint, s.Args);
                }

                // Add Admin facet
                _communicator.AddAdminFacet("IceBox.ServiceManager", this);

                // We can now activate the main communicator.
                await _communicator.ActivateAsync().ConfigureAwait(false);

                // We may want to notify external scripts that the services have started and that IceBox is "ready".
                // This is done by defining the property IceBox.PrintServicesReady=bundleName, bundleName is whatever
                // you choose to call this set of services. It will be echoed back as "bundleName ready".
                //
                // This must be done after start() has been invoked on the services.
                if (_communicator.GetProperty("IceBox.PrintServicesReady") is string bundleName)
                {
                    Console.Out.WriteLine($"{bundleName} ready");
                }

                await _communicator.WaitForShutdownAsync().ConfigureAwait(false);
            }
            catch (ObjectDisposedException)
            {
                // Expected if the communicator or ObjectAdater are disposed
            }
            catch (Exception ex)
            {
                _logger.Error($"IceBox.ServiceManager: caught exception:\n{ex}");
                return 1;
            }
            finally
            {
                StopAll();
            }

            return 0;
        }

        private bool ConfigureAdmin(Dictionary<string, string> properties, string prefix)
        {
            if (_adminEnabled && !properties.ContainsKey("Ice.Admin.Enabled"))
            {
                var facetNames = new List<string>();
                Debug.Assert(_adminFacetFilter != null);
                foreach (string p in _adminFacetFilter)
                {
                    if (p.StartsWith(prefix, StringComparison.InvariantCulture))
                    {
                        facetNames.Add(p.Substring(prefix.Length));
                    }
                }

                if (_adminFacetFilter.Count == 0 || facetNames.Count > 0)
                {
                    properties["Ice.Admin.Enabled"] = "1";

                    if (facetNames.Count > 0)
                    {
                        properties["Ice.Admin.Facets"] = StringUtil.ToPropertyValue(facetNames);
                    }
                    return true;
                }
            }
            return false;
        }

        private Dictionary<string, string> CreateServiceProperties(string service)
        {
            Dictionary<string, string> properties;
            if (_communicator.GetPropertyAsBool("IceBox.InheritProperties") ?? false)
            {
                // Inherit all except Ice.Admin.xxx properties
                properties = _communicator.GetProperties().Where(
                    p => !p.Key.StartsWith("Ice.Admin.", StringComparison.InvariantCulture)).ToDictionary(
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

        private void DestroyServiceCommunicator(string service, Communicator communicator)
        {
            if (communicator != null)
            {
                try
                {
                    communicator.ShutdownAsync().GetAwaiter().GetResult();
                }
                catch (CommunicatorDisposedException)
                {
                    // Ignore, the service might have already destroyed the communicator for its own reasons.
                }
                catch (Exception ex)
                {
                    _logger.Warning(@$"IceBox.ServiceManager: exception while shutting down communicator for service `{
                        service}'\n{ex}");
                }

                RemoveAdminFacets($"IceBox.Service.{service}.");
                communicator.Dispose();
            }
        }

        private void RemoveAdminFacets(string prefix)
        {
            try
            {
                foreach (string p in _communicator.FindAllAdminFacets().Keys)
                {
                    if (p.StartsWith(prefix, StringComparison.InvariantCulture))
                    {
                        _communicator.RemoveAdminFacet(p);
                    }
                }
            }
            catch (ObjectDisposedException)
            {
                // Expected if the communicator or ObjectAdapter are disposed.
            }
        }

        private void RemoveObserver(IServiceObserverPrx observer, Exception ex)
        {
            lock (_mutex)
            {
                if (_observers.Remove(observer))
                {
                    // CommunicatorDestroyedException may occur during shutdown. The observer notification has been sent,
                    // but the communicator was destroyed before the reply was received. We do not log a message for this
                    // exception.
                    if (_traceServiceObserver >= 1 && !(ex is CommunicatorDisposedException))
                    {
                        _logger.Trace("IceBox.ServiceObserver",
                                      $"Removed service observer {observer}\nafter catching {ex}");
                    }
                }
            }
        }

        private void ServiceStarted(string service, IEnumerable<IServiceObserverPrx> observers)
        {
            // Must be called with '_mutex' unlocked
            string[] services = new string[] { service };

            foreach (IServiceObserverPrx observer in observers)
            {
                _ = StartedAsync(observer, services);
            }

            async Task StartedAsync(IServiceObserverPrx observer, string[] services)
            {
                try
                {
                    await observer.ServicesStartedAsync(services).ConfigureAwait(false);
                }
                catch (Exception ex)
                {
                    RemoveObserver(observer, ex);
                }
            }
        }

        private void ServicesStopped(string[] services, IEnumerable<IServiceObserverPrx> observers)
        {
            // Must be called with '_mutex' unlocked
            if (services.Length > 0)
            {
                foreach (IServiceObserverPrx observer in observers)
                {
                    _ = StoppedAsync(observer, services);
                }

                async Task StoppedAsync(IServiceObserverPrx observer, string[] services)
                {
                    try
                    {
                        await observer.ServicesStoppedAsync(services).ConfigureAwait(false);
                    }
                    catch (Exception ex)
                    {
                        RemoveObserver(observer, ex);
                    }
                }
            }
        }

        private void StartService(string service, string entryPoint, string[] args)
        {
            lock (_mutex)
            {
                // Extract the assembly name and the class name.
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

                Assembly? serviceAssembly = null;
                string assemblyName = entryPoint.Substring(0, sepPos);
                string className = entryPoint.Substring(sepPos + 1);

                try
                {
                    // First try to load the assembly using Assembly.Load, which will succeed
                    // if a fully-qualified name is provided or if a partial name has been qualified
                    // in configuration. If that fails, try Assembly.LoadFrom(), which will succeed
                    // if a file name is configured or a partial name is configured and DEVPATH is used.
                    try
                    {
                        serviceAssembly = Assembly.Load(assemblyName);
                    }
                    catch (Exception)
                    {
                        try
                        {
                            serviceAssembly = Assembly.LoadFrom(assemblyName);
                        }
                        catch (Exception)
                        {
                        }

                        if (serviceAssembly == null)
                        {
                            throw;
                        }
                    }
                }
                catch (Exception ex)
                {
                    throw new LoadException(
                        @$"IceBox.ServiceManager: unable to load service `{entryPoint
                        }': error loading assembly `{assemblyName}'",
                        ex);
                }

                Type? c = null;
                try
                {
                    c = serviceAssembly.GetType(className, true);
                }
                catch (Exception ex)
                {
                    throw new LoadException(
                        @$"IceBox.ServiceManager: unable to load service `{entryPoint
                        }': cannot find the service class `{className}'",
                        ex);
                }
                Debug.Assert(c != null);

                var info = new ServiceInfo(service, ServiceStatus.Stopped, args);

                ILogger? logger = null;
                // If IceBox.UseSharedCommunicator.<name> is defined, create a communicator for the service. The
                // communicator inherits from the shared communicator properties. If it's not defined, add the
                // service properties to the shared communicator property set.
                Communicator communicator;
                if (_communicator.GetPropertyAsBool($"IceBox.UseSharedCommunicator.{service}") ?? false)
                {
                    Debug.Assert(_sharedCommunicator != null);
                    communicator = _sharedCommunicator;
                }
                else
                {
                    // Create the service properties. We use the communicator properties as the default properties if
                    // IceBox.InheritProperties is set.
                    Dictionary<string, string> properties = CreateServiceProperties(service);
                    if (info.Args.Length > 0)
                    {
                        // Create the service properties with the given service arguments. This should read the service
                        // configuration file if it's specified with --Ice.Config.
                        properties.ParseIceArgs(ref info.Args);

                        // Next, parse the service "<service>.*" command line options (the Ice command line options
                        // were parsed by the createProperties above)
                        properties.ParseArgs(ref info.Args, service);
                    }

                    // Clone the logger to assign a new prefix. If one of the built-in loggers is configured
                    // don't set any logger.
                    if (properties.TryGetValue("Ice.LogFile", out string? logFile))
                    {
                        logger = _logger.CloneWithPrefix(properties.GetValueOrDefault("Ice.ProgramName") ?? "");
                    }

                    // If Admin is enabled on the IceBox communicator, for each service that does not set
                    // Ice.Admin.Enabled, we set Ice.Admin.Enabled=1 to have this service create facets; then
                    // we add these facets to the IceBox Admin object as IceBox.Service.<service>.<facet>.
                    string serviceFacetNamePrefix = "IceBox.Service." + service + ".";
                    bool addFacets = ConfigureAdmin(properties, serviceFacetNamePrefix);

                    // Remaining command line options are passed to the communicator. This is necessary for Ice plug-in
                    // properties (e.g.: IceSSL).
                    info.Communicator = new Communicator(ref info.Args, properties, logger: logger);
                    communicator = info.Communicator;

                    if (addFacets)
                    {
                        // Add all facets created on the service communicator to the IceBox communicator
                        // but renamed IceBox.Service.<service>.<facet-name>, except for the Process facet
                        // which is never added
                        foreach (KeyValuePair<string, IObject> p in communicator.FindAllAdminFacets())
                        {
                            if (!p.Key.Equals("Process"))
                            {
                                _communicator.AddAdminFacet(serviceFacetNamePrefix + p.Key, p.Value);
                            }
                        }
                    }
                }

                try
                {
                    // Instantiate the service.
                    try
                    {
                        // If the service class provides a constructor that accepts an Ice.Communicator argument, use
                        // that in preference to the default constructor.
                        var parameterTypes = new Type[1] { typeof(Communicator) };
                        ConstructorInfo? ci = c.GetConstructor(parameterTypes);
                        if (ci != null)
                        {
                            info.Service = (IService)ci.Invoke(new object[1] { _communicator });
                        }
                        else
                        {
                            info.Service = (IService?)Activator.CreateInstance(c);
                        }
                    }
                    catch (Exception ex)
                    {
                        throw new LoadException($"IceBox.ServiceManager: unable to load service `{entryPoint}'", ex);
                    }
                    Debug.Assert(info.Service != null);
                    info.Service.Start(service, communicator, info.Args);
                    info.Status = ServiceStatus.Started;
                    _services.Add(info);
                }
                catch (Exception)
                {
                    if (info.Communicator != null)
                    {
                        DestroyServiceCommunicator(service, info.Communicator);
                    }
                    throw;
                }
            }
        }

        private void StopAll()
        {
            lock (_mutex)
            {
                // First wait for any active startService/stopService calls to complete.
                while (_pendingStatusChanges)
                {
                    Monitor.Wait(_mutex);
                }

                // For each service, we call stop on the service and flush its database environment to
                // the disk. Services are stopped in the reverse order of the order they were started.
                _services.Reverse();
                var stoppedServices = new List<string>();
                foreach (ServiceInfo info in _services)
                {
                    if (info.Status == ServiceStatus.Started)
                    {
                        try
                        {
                            Debug.Assert(info.Service != null);
                            info.Service.Stop();
                            stoppedServices.Add(info.Name);
                        }
                        catch (Exception ex)
                        {
                            _logger.Warning(
                                $"IceBox.ServiceManager: exception while stopping service {info.Name}:\n{ex}");
                        }
                    }

                    if (info.Communicator != null)
                    {
                        DestroyServiceCommunicator(info.Name, info.Communicator);
                    }
                }

                if (_sharedCommunicator != null)
                {
                    RemoveAdminFacets("IceBox.SharedCommunicator.");

                    try
                    {
                        _sharedCommunicator.Dispose();
                    }
                    catch (Exception ex)
                    {
                        _logger.Warning(
                            $"IceBox.ServiceManager: exception while destroying shared communicator:\n{ex}");
                    }
                    _sharedCommunicator = null;
                }

                _services.Clear();
                ServicesStopped(stoppedServices.ToArray(), _observers);
            }
        }

        private enum ServiceStatus : byte
        {
            Stopping,
            Stopped,
            Starting,
            Started
        }

        private class ServiceInfo
        {
            internal string[] Args;
            internal Communicator? Communicator;
            internal readonly string Name;
            internal IService? Service;
            internal ServiceStatus Status;

            internal ServiceInfo(string name, ServiceStatus status, string[] args)
            {
                Name = name;
                Status = status;
                Args = args;
            }
        }

        private class StartServiceInfo
        {
            internal string[] Args;
            internal string EntryPoint;
            internal string Name;

            internal StartServiceInfo(string service, string value, string[] serverArgs)
            {
                // Separate the entry point from the arguments.
                Name = service;

                try
                {
                    Args = Options.Split(value);
                }
                catch (FormatException ex)
                {
                    throw new ArgumentException($"IceBox.ServiceManager: invalid arguments for service `{Name}'", ex);
                }

                Debug.Assert(Args.Length > 0);

                EntryPoint = Args[0];
                Args = Args.Skip(1).Concat(
                    serverArgs.Where(
                        arg => arg.StartsWith($"--{service}.", StringComparison.InvariantCulture))).ToArray();
            }
        }
    }
}
