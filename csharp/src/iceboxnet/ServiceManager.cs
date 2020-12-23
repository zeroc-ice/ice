// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Collections.Immutable;
using System.Diagnostics;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

using ZeroC.Ice;

namespace ZeroC.IceBox
{
    internal sealed class ServiceManager : IAsyncServiceManager
    {
        private readonly HashSet<string>? _adminFacetFilter;
        private readonly string[] _argv; // Filtered server argument vector
        private readonly Communicator _communicator;

        private readonly ILogger _logger;
        private readonly object _mutex = new(); // protects _observers
        private readonly HashSet<IServiceObserverPrx> _observers = new();

        private readonly List<ServiceInfo> _services = new();
        private Communicator? _sharedCommunicator;

        private readonly int _traceServiceObserver;

        internal ServiceManager(Communicator communicator, string[] args)
        {
            _communicator = communicator;
            _logger = _communicator.Logger;

            bool adminEnabled = _communicator.GetPropertyAsBool("Ice.Admin.Enabled") ??
                _communicator.GetProperty("Ice.Admin.Endpoints") != null;

            if (adminEnabled)
            {
                _adminFacetFilter = new HashSet<string>(
                    _communicator.GetPropertyAsList("Ice.Admin.Facets") ?? Array.Empty<string>());
            }

            _argv = args;
            _traceServiceObserver = _communicator.GetPropertyAsInt("IceBox.Trace.ServiceObserver") ?? 0;
        }

        public ValueTask AddObserverAsync(IServiceObserverPrx observer, Current current, CancellationToken cancel)
        {
            lock (_mutex)
            {
                if (!_observers.Add(observer))
                {
                    return default; // ignore duplicates.
                }
            }

            if (_traceServiceObserver >= 1)
            {
                _logger.Trace("IceBox.ServiceObserver", $"added service observer {observer}");
            }

            IEnumerable<string> activeServices = _services.Where(info => info.IsActive).Select(info => info.Name);

            if (activeServices.Any())
            {
                _ = StartedAsync(observer, activeServices);
            }
            return default;

            async Task StartedAsync(IServiceObserverPrx observer, IEnumerable<string> services)
            {
                try
                {
                    // We don't want to forward the cancel of a dispatch that has already completed.
                    await observer.ServicesStartedAsync(services, cancel: default);
                }
                catch (Exception ex)
                {
                    RemoveObserver(observer, ex);
                }
            }
        }

        public ValueTask ShutdownAsync(Current current, CancellationToken cancel)
        {
            _ = _communicator.ShutdownAsync();
            return default;
        }

        public async ValueTask StartServiceAsync(string name, Current current, CancellationToken cancel)
        {
            // Search would be more efficient if services were contained in a map, but order is required for
            // shutdown.
            ServiceInfo? info = _services.Find(service => service.Name == name);
            if (info == null)
            {
                throw new NoSuchServiceException();
            }

            try
            {
                await info.StartServiceAsync(_sharedCommunicator, cancel);

                // We call ServiceStarted even if this call does not start the service - this is a way to refresh
                // the status of this service in the observers.
                ServiceStarted(name);
            }
            catch (Exception ex)
            {
                _logger.Warning($"IceBox.ServiceManager: exception while starting service `{name}':\n{ex}");
                throw new ServiceException($"service {name} failed to start", ex);
            }
        }

        public async ValueTask StopServiceAsync(string name, Current current, CancellationToken cancel)
        {
            ServiceInfo? info = _services.Find(service => service.Name == name);
            if (info == null)
            {
                throw new NoSuchServiceException();
            }

            try
            {
                await info.StopServiceAsync();

                // We notify the observers even if this call did not actually stop the service.
                ServiceStopped(name);
            }
            catch (Exception ex)
            {
                _logger.Warning($"IceBox.ServiceManager: exception while stopping service `{name}':\n{ex}");
                throw new ServiceException($"service {name} failed to stop", ex);
            }
        }

        internal async Task<int> RunAsync(CancellationToken cancel)
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

                string[] loadOrder =
                    (_communicator.GetPropertyAsList("IceBox.LoadOrder") ?? Array.Empty<string>()).Where(
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
                    await _sharedCommunicator.ActivateAsync(cancel);

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
                    await InitialStartServiceAsync(s.Name, s.EntryPoint, s.Args, cancel);
                }
                // From now on, _services is immutable.

                // Add Admin facet
                _communicator.AddAdminFacet("IceBox.ServiceManager", this);

                // We can now activate the main communicator.
                await _communicator.ActivateAsync(cancel);

                // We may want to notify external scripts that the services have started and that IceBox is "ready".
                // This is done by defining the property IceBox.PrintServicesReady=bundleName, bundleName is whatever
                // you choose to call this set of services. It will be echoed back as "bundleName ready".
                //
                // This must be done after start() has been invoked on the services.
                if (_communicator.GetProperty("IceBox.PrintServicesReady") is string bundleName)
                {
                    Console.Out.WriteLine($"{bundleName} ready");
                }

                await _communicator.WaitForShutdownAsync();
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
                await StopAllAsync();
            }

            return 0;
        }

        private bool ConfigureAdmin(Dictionary<string, string> properties, string prefix)
        {
            if (_adminFacetFilter != null && !properties.ContainsKey("Ice.Admin.Enabled"))
            {
                var facetNames = new List<string>();
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

        private async Task DestroyServiceCommunicatorAsync(string service, Communicator communicator)
        {
            try
            {
                await communicator.ShutdownAsync();
            }
            catch (CommunicatorDisposedException)
            {
                // Ignore, the service might have already destroyed the communicator for its own reasons.
            }
            catch (Exception ex)
            {
                var sb = new StringBuilder(
                    "IceBox.ServiceManager: exception while shutting down communicator for service `");
                sb.Append(service);
                sb.Append("'\n:");
                sb.Append(ex);
                _logger.Warning(sb.ToString());
            }

            RemoveAdminFacets($"IceBox.Service.{service}.");
            await communicator.DisposeAsync();
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
                    // CommunicatorDisposedException may occur during shutdown. The observer notification has been sent,
                    // but the communicator was destroyed before the reply was received. We do not log a message for this
                    // exception.
                    if (_traceServiceObserver >= 1 && !(ex is CommunicatorDisposedException))
                    {
                        _logger.Trace("IceBox.ServiceObserver",
                                      $"removed service observer {observer}\nafter catching {ex}");
                    }
                }
            }
        }

        private void ServiceStarted(string name)
        {
            var services = ImmutableList.Create(name);
            IServiceObserverPrx[] observers;
            lock (_mutex)
            {
                observers = _observers.ToArray();
            }
            foreach (IServiceObserverPrx observer in observers)
            {
                _ = StartedAsync(observer, services);
            }

            async Task StartedAsync(IServiceObserverPrx observer, IEnumerable<string> services)
            {
                try
                {
                    await observer.ServicesStartedAsync(services);
                }
                catch (Exception ex)
                {
                    RemoveObserver(observer, ex);
                }
            }
        }

        private void ServiceStopped(string name) => ServicesStopped(ImmutableList.Create(name));

        private void ServicesStopped(IEnumerable<string> services)
        {
            if (services.Any())
            {
                IServiceObserverPrx[] observers;
                lock (_mutex)
                {
                    observers = _observers.ToArray();
                }
                foreach (IServiceObserverPrx observer in observers)
                {
                    _ = StoppedAsync(observer, services);
                }
            }

            async Task StoppedAsync(IServiceObserverPrx observer, IEnumerable<string> services)
            {
                try
                {
                    await observer.ServicesStoppedAsync(services);
                }
                catch (Exception ex)
                {
                    RemoveObserver(observer, ex);
                }
            }
        }

        /// <summary>Starts a service during IceBox start-up. At this point, the service manager is not accessible to
        /// anyone else and no observer can be registered.</summary>
        private async Task InitialStartServiceAsync(
            string serviceName,
            string entryPoint,
            string[] args,
            CancellationToken cancel)
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

            ILogger? logger = null;
            // If IceBox.UseSharedCommunicator.<name> is defined, create a communicator for the service. The
            // communicator inherits from the shared communicator properties. If it's not defined, add the
            // service properties to the shared communicator property set.

            Communicator communicator;
            Communicator? serviceCommunicator = null;

            if (_communicator.GetPropertyAsBool($"IceBox.UseSharedCommunicator.{serviceName}") ?? false)
            {
                Debug.Assert(_sharedCommunicator != null);
                communicator = _sharedCommunicator;
            }
            else
            {
                // Create the service properties. We use the communicator properties as the default properties if
                // IceBox.InheritProperties is set.
                Dictionary<string, string> properties = CreateServiceProperties(serviceName);
                if (args.Length > 0)
                {
                    // Create the service properties with the given service arguments. This should read the service
                    // configuration file if it's specified with --Ice.Config.
                    properties.ParseIceArgs(ref args);

                    // Next, parse the service "<service>.*" command line options (the Ice command line options
                    // were parsed by the createProperties above)
                    properties.ParseArgs(ref args, serviceName);
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
                string serviceFacetNamePrefix = $"IceBox.Service.{serviceName}.";
                bool addFacets = ConfigureAdmin(properties, serviceFacetNamePrefix);

                // Remaining command line options are passed to the communicator. This is necessary for Ice plug-in
                // properties (e.g.: IceSSL).
                serviceCommunicator = new Communicator(ref args, properties, logger: logger);
                communicator = serviceCommunicator;

                try
                {
                    await serviceCommunicator.ActivateAsync(cancel);

                    if (addFacets)
                    {
                        // Add all facets created on the service communicator to the IceBox communicator
                        // but renamed IceBox.Service.<service>.<facet-name>, except for the Process facet
                        // which is never added.
                        foreach ((string facetName, IObject facet) in communicator.FindAllAdminFacets())
                        {
                            if (facetName != "Process")
                            {
                                _communicator.AddAdminFacet($"{serviceFacetNamePrefix}{facetName}", facet);
                            }
                        }
                    }
                }
                catch
                {
                    await serviceCommunicator.DisposeAsync();
                    throw; // errors during this initial start are fatal
                }
            }

            IService? service;
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
                        service = (IService)ci.Invoke(new object[1] { _communicator });
                    }
                    else
                    {
                        service = (IService?)Activator.CreateInstance(c);
                    }
                }
                catch (Exception ex)
                {
                    throw new LoadException($"IceBox.ServiceManager: unable to load service `{entryPoint}'", ex);
                }

                Debug.Assert(service != null);
                await service.StartAsync(serviceName, communicator, args, cancel);
            }
            catch
            {
                if (serviceCommunicator != null)
                {
                    await DestroyServiceCommunicatorAsync(serviceName, serviceCommunicator);
                }
                throw;
            }

            _services.Add(new ServiceInfo(serviceName, service, serviceCommunicator, args));
        }

        private async Task StopAllAsync()
        {
            // When this method executes, the server is fully shut down, so no concurrent dispatch to StartServiceAsync
            // etc.

            // For each service, we call stop on the service and flush its database environment to the disk. Services
            // are stopped in the reverse order of the order they were started.
            _services.Reverse();
            var stoppedServices = new List<string>();
            foreach (ServiceInfo info in _services)
            {
                try
                {
                    await info.StopServiceAsync();
                    stoppedServices.Add(info.Name);
                }
                catch (Exception ex)
                {
                    _logger.Warning($"IceBox.ServiceManager: exception while stopping service {info.Name}:\n{ex}");
                }

                if (info.Communicator is Communicator serviceCommunicator)
                {
                    await DestroyServiceCommunicatorAsync(info.Name, serviceCommunicator);
                }
            }

            if (_sharedCommunicator is Communicator sharedCommunicator)
            {
                RemoveAdminFacets("IceBox.SharedCommunicator.");

                try
                {
                    await sharedCommunicator.DisposeAsync();
                }
                catch (Exception ex)
                {
                    _logger.Warning($"IceBox.ServiceManager: exception while destroying shared communicator:\n{ex}");
                }
            }
            ServicesStopped(stoppedServices);
        }

        private class ServiceInfo
        {
            internal Communicator? Communicator { get; } // null means uses shared communicator

            internal bool IsActive
            {
                get
                {
                    lock (_mutex)
                    {
                        if (_startTask is Lazy<Task> startTask &&
                            startTask.IsValueCreated &&
                            startTask.Value.IsCompletedSuccessfully)
                        {
                            return true;
                        }
                        else if (_stopTask is Lazy<Task> stopTask &&
                                (!stopTask.IsValueCreated || !stopTask.Value.IsCompletedSuccessfully))
                        {
                            return true;
                        }
                        else
                        {
                            return false;
                        }
                    }
                }
            }

            internal string Name { get; }

            private readonly string[] _args;

            private readonly object _mutex = new(); // protects _startTask and _stopTask

            private readonly IService _service;

            // When _startTask is not null, the service is starting, started (_startTask completed successfully) or
            // stopped (_startTask is failed or canceled).
            private Lazy<Task>? _startTask;

            // When _stopTask is not null, the service is stopping, stopped or started (_stopTask is failed or
            // canceled).
            private Lazy<Task>? _stopTask;

            internal ServiceInfo(string name, IService service, Communicator? communicator, string[] args)
            {
                _args = args;
                Communicator = communicator;
                Name = name;
                _service = service;
                _startTask = new(Task.CompletedTask); // ServiceInfo is created after the initial successful start
            }

            internal async Task StartServiceAsync(Communicator? sharedCommunicator, CancellationToken cancel)
            {
                // We use lazy tasks to avoid calling Service.StartAsync and Service.StopAsync with _mutex locked
                // because StartAsync/StopAsync can potentially acquire synchronously other mutexes and create an
                // out-of-order lock acquisition deadlocks.

                Lazy<Task>? performTask = null;

                do
                {
                    // We may need to wait until the current stop task completes.
                    Lazy<Task>? waitTask = null;

                    lock (_mutex)
                    {
                        if (_stopTask is Lazy<Task> stopTask)
                        {
                            Debug.Assert(_startTask == null);
                            if (!stopTask.IsValueCreated)
                            {
                                waitTask = stopTask;
                            }
                            else if (stopTask.Value.IsFaulted || stopTask.Value.IsCanceled)
                            {
                                // Already started, no need to do anything.
                                _stopTask = null;
                                _startTask = new(Task.CompletedTask);
                                performTask = _startTask;
                            }
                            else if (!stopTask.Value.IsCompletedSuccessfully)
                            {
                                waitTask = stopTask; // we need to wait until this task is complete
                            }
                            // else stopTask completed successfully and we replace it with a new startTask below.
                        }
                        else if (_startTask is Lazy<Task> startTask)
                        {
                            if (!startTask.IsValueCreated ||
                                startTask.Value.IsCompletedSuccessfully ||
                                !startTask.Value.IsCompleted)
                            {
                                performTask = startTask;
                            }
                            // else, startTask is canceled or faulted and we create a new startTask below.
                        }
                        else
                        {
                            Debug.Assert(false);
                            performTask = new(Task.CompletedTask); // never reached
                        }

                        if (waitTask == null && performTask == null)
                        {
                            _stopTask = null;

                            // The async/await ensures we get a Task (i.e. successful lazy initialization) even if
                            // StartAsync throws an exception synchronously.
                            // Note that StartAsync is not executed immediately with _mutex locked.
                            _startTask = new(
                                    async () => await _service.StartAsync(
                                                    Name,
                                                    Communicator ?? sharedCommunicator!,
                                                    _args,
                                                    cancel).ConfigureAwait(false));

                            performTask = _startTask;
                        }
                    }

                    if (waitTask != null)
                    {
                        try
                        {
                            await waitTask.Value.WaitAsync(cancel);
                        }
                        catch
                        {
                            // If the await failed because this cancel was canceled, we want to throw, else we ignore
                            // the exception and loop back.
                            cancel.ThrowIfCancellationRequested();
                        }
                    }
                }
                while (performTask == null);

                await performTask.Value.WaitAsync(cancel);
            }

            internal async Task StopServiceAsync()
            {
                Lazy<Task>? performTask = null;

                do
                {
                    // We may need to wait until the current start task completes.
                    Lazy<Task>? waitTask = null;

                    lock (_mutex)
                    {
                        if (_stopTask is Lazy<Task> stopTask)
                        {
                            Debug.Assert(_startTask == null);
                            if (!stopTask.IsValueCreated ||
                                stopTask.Value.IsCompletedSuccessfully ||
                                !stopTask.Value.IsCompleted)
                            {
                                performTask = stopTask;
                            }
                            // else, stopTask is canceled or faulted and we create a new stopTask below.
                        }
                        else if (_startTask is Lazy<Task> startTask)
                        {
                            if (!startTask.IsValueCreated)
                            {
                                waitTask = startTask;
                            }
                            else if (startTask.Value.IsFaulted || startTask.Value.IsCanceled)
                            {
                                // Already stopped, no need to do anything.
                                _startTask = null;
                                _stopTask = new(Task.CompletedTask);
                                performTask = _stopTask;
                            }
                            else if (!startTask.Value.IsCompletedSuccessfully)
                            {
                                waitTask = startTask; // we need to wait until this task is complete
                            }
                            // else, startTask completed successfully and we replace it with a stopTask below.
                        }
                        else
                        {
                            Debug.Assert(false);
                            performTask = new(Task.CompletedTask);
                        }

                        if (waitTask == null && performTask == null)
                        {
                            _startTask = null;

                            // The async/await ensures we get a Task (i.e. successful lazy initialization) even if
                            // StopAsync throws an exception synchronously.
                            // Note that StopAsync is not executed immediately with _mutex locked.
                            _stopTask = new(async () => await _service.StopAsync());
                            performTask = _stopTask;
                        }
                    }

                    if (waitTask != null)
                    {
                        try
                        {
                            await waitTask.Value;
                        }
                        catch
                        {
                            // ignore exception
                        }
                    }
                }
                while (performTask == null);

                await performTask.Value;
            }
        }

        private class StartServiceInfo
        {
            internal string[] Args;
            internal string EntryPoint { get; }
            internal string Name { get; }

            internal StartServiceInfo(string serviceName, string value, string[] serverArgs)
            {
                // Separate the entry point from the arguments.
                Name = serviceName;

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
                        arg => arg.StartsWith($"--{serviceName}.", StringComparison.InvariantCulture))).ToArray();
            }
        }
    }
}
