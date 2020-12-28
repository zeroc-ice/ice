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

                if (_traceServiceObserver >= 1)
                {
                    _logger.Trace("IceBox.ServiceObserver", $"added service observer {observer}");
                }

                // Important: IsStarted is called (possibly lazily) with _mutex locked.

                IEnumerable<string> startedServices = _services.Where(info => info.IsStarted).Select(info => info.Name);

                if (startedServices.Any())
                {
                    _ = StartedAsync(observer, startedServices);
                }
                return default;
            }

            async Task StartedAsync(IServiceObserverPrx observer, IEnumerable<string> services)
            {
                try
                {
                    // We don't want to forward the cancel of a dispatch that has already completed.
                    // The sending / queueing is done with _mutex locked, so barring a connection failure + retry, it
                    // will reach the observer before subsequent state change notifications.
                    // Note: _mutex is locked when services is marshaled, which is necessary in case IsStarted is only
                    // evaluated at that time.
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
            if (_services.Find(service => service.Name == name) is ServiceInfo info)
            {
                await info.StartAsync(this, cancel);
            }
            else
            {
                throw new NoSuchServiceException();
            }
        }

        public async ValueTask StopServiceAsync(string name, Current current, CancellationToken cancel)
        {
            if (_services.Find(service => service.Name == name) is ServiceInfo info)
            {
                await info.StopAsync(this, notifyObservers: true);
            }
            else
            {
                throw new NoSuchServiceException();
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
            catch (OperationCanceledException)
            {
                // Expected if ActivateAsync is canceled through Ctrl+C.
            }
            catch (Exception ex)
            {
                _logger.Error($"IceBox.ServiceManager: caught exception:\n{ex}");
                return 1;
            }
            finally
            {
                await _communicator.ShutdownAsync(); // often no-op since the communicator is usually already shut down.

                // When this code executes, the server is fully shut down, so no concurrent dispatch to
                // StartServiceAsync etc.

                // For each service, we call stop on the service and flush its database environment to the disk.
                // Services are stopped in the reverse order of the order they were started.
                _services.Reverse();
                var stoppedServices = new List<string>();
                foreach (ServiceInfo info in _services)
                {
                    try
                    {
                        await info.StopAsync(this, notifyObservers: false);
                    }
                    catch
                    {
                        // ignore
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
                        _logger.Warning(
                            $"IceBox.ServiceManager: exception while destroying shared communicator:\n{ex}");
                    }
                }
                ServicesStopped(stoppedServices);
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
            lock (_mutex)
            {
                // We serialize all notifications (by locking _mutex) to ensure proper ordering.
                foreach (IServiceObserverPrx observer in _observers)
                {
                    _ = StartedAsync(observer, services);
                }
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

        private void ServicesStopped(ICollection<string> services)
        {
            if (services.Count > 0)
            {
                lock (_mutex)
                {
                    // We serialize all notifications (by locking _mutex) to ensure proper ordering.
                    foreach (IServiceObserverPrx observer in _observers)
                    {
                        _ = StoppedAsync(observer, services);
                    }
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

        private class ServiceInfo
        {
            internal Communicator? Communicator { get; } // null means uses shared communicator

            /// <summary>Return true when the service is started, and returns false when the service is stopped. Other
            /// states are not visible to the caller.
            /// IMPORTANT: This property must be called with ServiceManager's _mutex locked!</summary>
            internal bool IsStarted
            {
                get
                {
                    if (_pendingTask is Task pendingTask)
                    {
                        // We clear _pendingTask if it's completed; when get returns, _pendingTask is null or still
                        // running.

                        if (pendingTask.IsCompletedSuccessfully)
                        {
                            _started = !_started;
                            _pendingTask = null;
                        }
                        else if (_pendingTask.IsFaulted || _pendingTask.IsCanceled)
                        {
                            _pendingTask = null; // _started remains the same
                        }
                        // else _pendingTask is still running and we're still in the old state
                    }
                    return _started;
                }
            }

            internal string Name { get; }

            private readonly string[] _args;

            // A task that is changing _started to its opposite value.
            private Task? _pendingTask;

            private readonly IService _service;

            private bool _started;

            internal ServiceInfo(string name, IService service, Communicator? communicator, string[] args)
            {
                _args = args;
                Communicator = communicator;
                Name = name;
                _service = service;
                _started = true;
            }

            /// <summary>Starts this service.</summary>
            /// <param name="serviceManager">The service manager.</param>
            /// <param name="cancel">The cancellation token.</param>
            /// <return>A task that completes successfully when the service is fully started.</return>
            internal async Task StartAsync(ServiceManager serviceManager, CancellationToken cancel)
            {
                Task? startTask = null;

                do
                {
                    // We may need to wait until the pending stop task completes
                    Task? stopTask = null;

                    lock (serviceManager._mutex)
                    {
                        if (IsStarted)
                        {
                            if (_pendingTask is Task pendingTask)
                            {
                                // It's currently being stopped, we are going to wait for stop to complete.
                                stopTask = pendingTask;
                            }
                            else
                            {
                                // already started
                                startTask = Task.CompletedTask;
                            }
                        }
                        else
                        {
                            // If it's currently being started, we piggy-back on this start, else we create a new start
                            // task.
                            _pendingTask ??= PerformStartAsync();
                            startTask = _pendingTask;
                        }
                    }

                    if (stopTask != null)
                    {
                        try
                        {
                            await stopTask.WaitAsync(cancel);
                        }
                        catch
                        {
                            // If the await failed because this cancel was canceled, we want to throw, else we ignore
                            // the exception and loop back.
                            cancel.ThrowIfCancellationRequested();
                        }
                    }
                }
                while (startTask == null);

                await startTask.WaitAsync(cancel);

                async Task PerformStartAsync()
                {
                    // Make sure we execute _service.StartAsync asynchronously because we don't want to call it while
                    // holding a lock on serviceManager._mutex.
                    await Task.Yield();

                    try
                    {
                        await _service.StartAsync(
                            Name,
                            Communicator ?? serviceManager._sharedCommunicator!,
                            _args,
                            cancel);
                    }
                    catch (Exception ex)
                    {
                        serviceManager._logger.Warning(
                            $"IceBox.ServiceManager: exception while starting service `{Name}':\n{ex}");
                        throw new ServiceException($"service {Name} failed to start", ex);
                    }

                    // We send the notification (without waiting for the response) just before the task completes
                    // successfully, to ensure proper ordering of notifications. While the task is still running, it
                    // cannot get replaced by a new task.
                    serviceManager.ServiceStarted(Name);
                }
            }

            /// <summary>Stops this service.</summary>
            /// <param name="serviceManager">The service manager.</param>
            /// <param name="notifyObservers">When true, the new stop task notifies the observers when the service is
            /// stopped; when false, the new stop task does not notify observers.</param>
            /// <return>A task that completes successfully when the service is fully stopped.</return>
            internal async Task StopAsync(ServiceManager serviceManager, bool notifyObservers)
            {
                Task? stopTask = null;

                do
                {
                    // We may need to wait until the current start task completes.
                    Task? startTask = null;

                    lock (serviceManager._mutex)
                    {
                        if (IsStarted)
                        {
                            // If it's currently being stopped, we piggy-back on this stop, else we create a new stop
                            // task.
                            _pendingTask ??= PerformStopAsync();
                            stopTask = _pendingTask;
                        }
                        else
                        {
                            if (_pendingTask is Task pendingTask)
                            {
                                // It's currently being started, we wait for this start to complete.
                                startTask = pendingTask;
                            }
                            else
                            {
                                // It's already stopped, nothing more to do.
                                stopTask = Task.CompletedTask;
                            }
                        }
                    }

                    if (startTask != null)
                    {
                        try
                        {
                            await startTask;
                        }
                        catch
                        {
                            // ignore exception
                        }
                    }
                }
                while (stopTask == null);

                await stopTask;

                async Task PerformStopAsync()
                {
                    // Make sure we execute _service.StopAsync asynchronously because we don't want to call it while
                    // holding a lock on serviceManager._mutex.
                    await Task.Yield();

                    try
                    {
                        await _service.StopAsync();
                    }
                    catch (Exception ex)
                    {
                        serviceManager._logger.Warning(
                            $"IceBox.ServiceManager: exception while stopping service `{Name}':\n{ex}");
                        throw new ServiceException($"service {Name} failed to stop", ex);
                    }

                    if (notifyObservers)
                    {
                        // We send the notification (without waiting for the response) before the task completes
                        // successfully, to ensure proper ordering of notifications. While the task is still running, it
                        // cannot get replaced by a new task.
                        serviceManager.ServiceStopped(Name);
                    }
                }
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
