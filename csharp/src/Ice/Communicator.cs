//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using IceInternal;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Diagnostics;
using System.Globalization;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading;

namespace Ice
{
    internal sealed class BufSizeWarnInfo
    {
        // Whether send size warning has been emitted
        public bool SndWarn;

        // The send size for which the warning was emitted
        public int SndSize;

        // Whether receive size warning has been emitted
        public bool RcvWarn;

        // The receive size for which the warning was emitted
        public int RcvSize;
    }

    public sealed partial class Communicator : IDisposable
    {
        private class ObserverUpdater : Instrumentation.IObserverUpdater
        {
            public ObserverUpdater(Communicator communicator) => _communicator = communicator;

            public void UpdateConnectionObservers() => _communicator.UpdateConnectionObservers();

            public void UpdateThreadObservers() => _communicator.UpdateThreadObservers();

            private readonly Communicator _communicator;
        }

        /// <summary>Indicates whether or not object adapters created by this communicator accept non-secure incoming
        /// connections. When false, they accept only secure connections; when true, they accept both secure and
        /// non-secure connections. This property corresponds to the Ice.AcceptNonSecureConnections configuration
        /// property. It can be overridden for each object adapter by the object adapter property with the same name.
        /// </summary>
        // TODO: update doc with default value for AcceptNonSecureConnections - it's currently true but should be
        // false.
        // TODO: currently only this property is implemented and nothing else.
        public bool AcceptNonSecureConnections { get; }

        /// <summary>Each time you send a request without an explicit context parameter, Ice sends automatically the
        /// per-thread CurrentContext combined with the proxy's context.</summary>
        public Dictionary<string, string> CurrentContext
        {
            get
            {
                try
                {
                    if (_currentContext.IsValueCreated)
                    {
                        Debug.Assert(_currentContext.Value != null);
                        return _currentContext.Value;
                    }
                    else
                    {
                        _currentContext.Value = new Dictionary<string, string>();
                        return _currentContext.Value;
                    }
                }
                catch (ObjectDisposedException ex)
                {
#pragma warning disable CA1065
                    throw new CommunicatorDestroyedException(ex);
#pragma warning restore CA1065
                }
            }
            set
            {
                try
                {
                    _currentContext.Value = value;
                }
                catch (ObjectDisposedException ex)
                {
                    throw new CommunicatorDestroyedException(ex);
                }
            }
        }

        public bool DefaultCollocationOptimized { get; }

        /// <summary>The default context for proxies created using this communicator. Changing the value of
        /// DefaultContext does not change the context of previously created proxies.</summary>
        public IReadOnlyDictionary<string, string> DefaultContext
        {
            get => _defaultContext;
            set => _defaultContext = value.Count == 0 ? Reference.EmptyContext : new Dictionary<string, string>(value);
        }

        public Encoding DefaultEncoding { get; }
        public EndpointSelectionType DefaultEndpointSelection { get; }
        public FormatType DefaultFormat { get; }
        public string? DefaultHost { get; }

        /// <summary>The default locator for this communicator. To disable the default locator, null can be used.
        /// All newly created proxies and object adapters will use this default locator. Note that setting this property
        /// has no effect on existing proxies or object adapters.</summary>
        public ILocatorPrx? DefaultLocator
        {
            get => _defaultLocator;
            set => _defaultLocator = value;
        }

        public bool DefaultPreferNonSecure { get; }
        public IPAddress? DefaultSourceAddress { get; }
        public string DefaultTransport { get; }
        public int DefaultTimeout { get; }
        public int DefaultInvocationTimeout { get; }
        public int DefaultLocatorCacheTimeout { get; }

        /// <summary>The default router for this communicator. To disable the default router, null can be used.
        /// All newly created proxies will use this default router. Note that setting this property has no effect on
        /// existing proxies.</summary>
        public IRouterPrx? DefaultRouter
        {
            get => _defaultRouter;
            set => _defaultRouter = value;
        }

        public bool? OverrideCompress { get; }
        public int? OverrideTimeout { get; }
        public int? OverrideCloseTimeout { get; }
        public int? OverrideConnectTimeout { get; }

        /// <summary>The logger for this communicator.</summary>
        public ILogger Logger { get; internal set; }

        public Instrumentation.ICommunicatorObserver? Observer { get; }

        public Action? ThreadStart { get; private set; }

        public Action? ThreadStop { get; private set; }

        public ToStringMode ToStringMode { get; }

        internal int ClassGraphDepthMax { get; }
        internal ACMConfig ClientACM { get; }
        internal int MessageSizeMax { get; }
        internal INetworkProxy? NetworkProxy { get; }
        internal bool PreferIPv6 { get; }
        internal int IPVersion { get; }
        internal ACMConfig ServerACM { get; }
        internal TraceLevels TraceLevels { get; private set; }

        private static string[] _emptyArgs = Array.Empty<string>();
        private static readonly string[] _suffixes =
        {
            "EndpointSelection",
            "ConnectionCached",
            "PreferNonSecure",
            "LocatorCacheTimeout",
            "InvocationTimeout",
            "Locator",
            "Router",
            "CollocationOptimized",
            "Context\\..*"
        };
        private static readonly object _staticLock = new object();

        private const int StateActive = 0;
        private const int StateDestroyInProgress = 1;
        private const int StateDestroyed = 2;

        private readonly HashSet<string> _adapterNamesInUse = new HashSet<string>();
        private readonly List<ObjectAdapter> _adapters = new List<ObjectAdapter>();
        private ObjectAdapter? _adminAdapter;
        private readonly bool _adminEnabled = false;
        private readonly HashSet<string> _adminFacetFilter = new HashSet<string>();
        private readonly Dictionary<string, IObject> _adminFacets = new Dictionary<string, IObject>();
        private Identity? _adminIdentity;
        private AsyncIOThread? _asyncIOThread;
        private readonly ConcurrentDictionary<string, IClassFactory?> _classFactoryCache =
            new ConcurrentDictionary<string, IClassFactory?>();
        private readonly string[] _classFactoryNamespaces;
        private readonly IceInternal.ThreadPool _clientThreadPool;
        private readonly ConcurrentDictionary<int, IClassFactory?> _compactIdCache =
            new ConcurrentDictionary<int, IClassFactory?>();
        private readonly ThreadLocal<Dictionary<string, string>> _currentContext
            = new ThreadLocal<Dictionary<string, string>>();
        private volatile IReadOnlyDictionary<string, string> _defaultContext = Reference.EmptyContext;
        private volatile ILocatorPrx? _defaultLocator;
        private volatile IRouterPrx? _defaultRouter;

        private bool _isShutdown = false;
        private static bool _oneOffDone = false;
        private readonly OutgoingConnectionFactory _outgoingConnectionFactory;
        private static bool _printProcessIdDone = false;
        private readonly ConcurrentDictionary<string, IRemoteExceptionFactory?> _remoteExceptionFactoryCache =
            new ConcurrentDictionary<string, IRemoteExceptionFactory?>();
        private readonly string[] _remoteExceptionFactoryNamespaces;
        private readonly int[] _retryIntervals;
        private IceInternal.ThreadPool? _serverThreadPool;
        private readonly Dictionary<EndpointType, BufSizeWarnInfo> _setBufSizeWarn =
            new Dictionary<EndpointType, BufSizeWarnInfo>();
        private int _state;
        private readonly IceInternal.Timer _timer;

        private readonly IDictionary<string, IEndpointFactory> _transportToEndpointFactory =
            new ConcurrentDictionary<string, IEndpointFactory>();

        private readonly IDictionary<EndpointType, IEndpointFactory> _typeToEndpointFactory =
            new ConcurrentDictionary<EndpointType, IEndpointFactory>();

        public Communicator(Dictionary<string, string>? properties,
                            ILogger? logger = null,
                            Instrumentation.ICommunicatorObserver? observer = null,
                            Action? threadStart = null,
                            Action? threadStop = null,
                            string[]? typeIdNamespaces = null)
            : this(ref _emptyArgs,
                   null,
                   properties,
                   logger,
                   observer,
                   threadStart,
                   threadStop,
                   typeIdNamespaces)
        {
        }

        public Communicator(ref string[] args,
                            Dictionary<string, string>? properties,
                            ILogger? logger = null,
                            Instrumentation.ICommunicatorObserver? observer = null,
                            Action? threadStart = null,
                            Action? threadStop = null,
                            string[]? typeIdNamespaces = null)
            : this(ref args,
                   null,
                   properties,
                   logger,
                   observer,
                   threadStart,
                   threadStop,
                   typeIdNamespaces)
        {
        }

        public Communicator(NameValueCollection? appSettings = null,
                            Dictionary<string, string>? properties = null,
                            ILogger? logger = null,
                            Instrumentation.ICommunicatorObserver? observer = null,
                            Action? threadStart = null,
                            Action? threadStop = null,
                            string[]? typeIdNamespaces = null)
            : this(ref _emptyArgs,
                   appSettings,
                   properties,
                   logger,
                   observer,
                   threadStart,
                   threadStop,
                   typeIdNamespaces)
        {
        }

        public Communicator(ref string[] args,
                            NameValueCollection? appSettings,
                            Dictionary<string, string>? properties = null,
                            ILogger? logger = null,
                            Instrumentation.ICommunicatorObserver? observer = null,
                            Action? threadStart = null,
                            Action? threadStop = null,
                            string[]? typeIdNamespaces = null)
        {
            _state = StateActive;
            Logger = logger ?? Util.GetProcessLogger();
            Observer = observer;
            ThreadStart = threadStart;
            ThreadStop = threadStop;

            _classFactoryNamespaces = new string[] { "Ice.ClassFactory" };
            _remoteExceptionFactoryNamespaces = new string[] { "Ice.RemoteExceptionFactory" };
            if (typeIdNamespaces != null)
            {
                _classFactoryNamespaces = _classFactoryNamespaces.Concat(typeIdNamespaces).ToArray();
                _remoteExceptionFactoryNamespaces =
                    _remoteExceptionFactoryNamespaces.Concat(typeIdNamespaces).ToArray();
            }

            if (properties == null)
            {
                properties = new Dictionary<string, string>();
            }
            else
            {
                // clone properties as we don't want to modify the properties given to
                // this constructor
                properties = new Dictionary<string, string>(properties);
            }

            if (appSettings != null)
            {
                foreach (string key in appSettings.AllKeys)
                {
                    string[]? values = appSettings.GetValues(key);
                    if (values == null)
                    {
                        properties[key] = "";
                    }
                    else
                    {
                        // TODO: this join is not sufficient to create a string
                        // compatible with GetPropertyAsList
                        properties[key] = string.Join(",", values);
                    }
                }
            }

            if (!properties.ContainsKey("Ice.ProgramName"))
            {
                properties["Ice.ProgramName"] = AppDomain.CurrentDomain.FriendlyName;
            }

            properties.ParseIceArgs(ref args);
            SetProperties(properties);

            try
            {
                lock (_staticLock)
                {
                    if (!_oneOffDone)
                    {
                        string? stdOut = GetProperty("Ice.StdOut");

                        System.IO.StreamWriter? outStream = null;

                        if (stdOut != null)
                        {
                            outStream = System.IO.File.AppendText(stdOut);
                            outStream.AutoFlush = true;
                            Console.Out.Close();
                            Console.SetOut(outStream);
                        }

                        string? stdErr = GetProperty("Ice.StdErr");
                        if (stdErr != null)
                        {
                            if (stdErr.Equals(stdOut))
                            {
                                Console.SetError(outStream);
                            }
                            else
                            {
                                System.IO.StreamWriter errStream = System.IO.File.AppendText(stdErr);
                                errStream.AutoFlush = true;
                                Console.Error.Close();
                                Console.SetError(errStream);
                            }
                        }

                        _oneOffDone = true;
                    }
                }

                if (logger == null)
                {
                    string? logfile = GetProperty("Ice.LogFile");
                    string? programName = GetProperty("Ice.ProgramName");
                    Debug.Assert(programName != null);
                    if (logfile != null)
                    {
                        Logger = new FileLogger(programName, logfile);
                    }
                    else if (Util.GetProcessLogger() is Logger)
                    {
                        //
                        // Ice.ConsoleListener is enabled by default.
                        //
                        Logger = new TraceLogger(programName, GetPropertyAsBool("Ice.ConsoleListener") ?? true);
                    }
                    // else already set to process logger
                }

                TraceLevels = new TraceLevels(this);

                DefaultCollocationOptimized = GetPropertyAsBool("Ice.Default.CollocationOptimized") ?? true;

                if (GetProperty("Ice.Default.Encoding") is string encoding)
                {
                    try
                    {
                        DefaultEncoding = Encoding.Parse(encoding);
                        DefaultEncoding.CheckSupported();
                    }
                    catch (Exception ex)
                    {
                        throw new InvalidConfigurationException(
                            $"invalid value for for Ice.Default.Encoding: `{encoding}'", ex);
                    }
                }
                else
                {
                    DefaultEncoding = Encoding.Latest;
                }

                string endpointSelection = GetProperty("Ice.Default.EndpointSelection") ?? "Random";
                DefaultEndpointSelection = endpointSelection switch
                {
                    "Random" => EndpointSelectionType.Random,
                    "Ordered" => EndpointSelectionType.Ordered,
                    _ => throw new InvalidConfigurationException(
                             $"illegal value `{endpointSelection}'; expected `Random' or `Ordered'")
                };

                DefaultFormat = (GetPropertyAsBool("Ice.Default.SlicedFormat") ?? false) ?
                    FormatType.Sliced : FormatType.Compact;

                DefaultHost = GetProperty("Ice.Default.Host");

                // TODO: switch to 0/false default
                DefaultPreferNonSecure = GetPropertyAsBool("Ice.Default.PreferNonSecure") ?? true;

                if (GetProperty("Ice.Default.SourceAddress") is string address)
                {
                    DefaultSourceAddress = Network.GetNumericAddress(address);
                    if (DefaultSourceAddress == null)
                    {
                        throw new InvalidConfigurationException(
                            $"invalid IP address set for Ice.Default.SourceAddress: `{address}'");
                    }
                }

                DefaultTransport = GetProperty("Ice.Default.Transport") ?? "tcp";

                DefaultTimeout = GetPropertyAsInt("Ice.Default.Timeout") ?? 60000;
                if (DefaultTimeout < 1 && DefaultTimeout != -1)
                {
                    throw new InvalidConfigurationException(
                        $"invalid value for Ice.Default.Timeout: `{DefaultTimeout}'");
                }

                DefaultInvocationTimeout = GetPropertyAsInt("Ice.Default.InvocationTimeout") ?? -1;
                if (DefaultInvocationTimeout < 1 && DefaultInvocationTimeout != -1)
                {
                    throw new InvalidConfigurationException(
                        $"invalid value for Ice.Default.InvocationTimeout: `{DefaultInvocationTimeout}'");
                }

                DefaultLocatorCacheTimeout = GetPropertyAsInt("Ice.Default.LocatorCacheTimeout") ?? -1;
                if (DefaultLocatorCacheTimeout < -1)
                {
                    throw new InvalidConfigurationException(
                        $"invalid value for Ice.Default.LocatorCacheTimeout: `{DefaultLocatorCacheTimeout}'");
                }

                if (GetPropertyAsBool("Ice.Override.Compress") is bool compress)
                {
                    OverrideCompress = compress;
                    if (!BZip2.IsLoaded && compress)
                    {
                        throw new InvalidConfigurationException($"compression not supported, bzip2 library not found");
                    }
                }
                else if (!BZip2.IsLoaded)
                {
                    OverrideCompress = false;
                }

                {
                    if (GetPropertyAsInt("Ice.Override.Timeout") is int timeout)
                    {
                        OverrideTimeout = timeout;
                        if (timeout < 1 && timeout != -1)
                        {
                            throw new InvalidConfigurationException(
                                $"invalid value for Ice.Override.Timeout: `{timeout}'");
                        }
                    }
                }

                {
                    if (GetPropertyAsInt("Ice.Override.CloseTimeout") is int timeout)
                    {
                        OverrideCloseTimeout = timeout;
                        if (timeout < 1 && timeout != -1)
                        {
                            throw new InvalidConfigurationException(
                                $"invalid value for Ice.Override.CloseTimeout: `{timeout}'");
                        }
                    }
                }

                {
                    if (GetPropertyAsInt("Ice.Override.ConnectTimeout") is int timeout)
                    {
                        OverrideConnectTimeout = timeout;
                        if (timeout < 1 && timeout != -1)
                        {
                            throw new InvalidConfigurationException(
                                $"invalid value for Ice.Override.ConnectTimeout: `{timeout}'");
                        }
                    }
                }

                ClientACM = new ACMConfig(this, Logger, "Ice.ACM.Client",
                                           new ACMConfig(this, Logger, "Ice.ACM", new ACMConfig(false)));

                ServerACM = new ACMConfig(this, Logger, "Ice.ACM.Server",
                                           new ACMConfig(this, Logger, "Ice.ACM", new ACMConfig(true)));

                {
                    int num = GetPropertyAsInt("Ice.MessageSizeMax") ?? 1024;
                    if (num < 1 || num > 0x7fffffff / 1024)
                    {
                        MessageSizeMax = 0x7fffffff;
                    }
                    else
                    {
                        MessageSizeMax = num * 1024; // Property is in kilobytes, MessageSizeMax in bytes
                    }
                }

                // TODO: switch to 0 default
                AcceptNonSecureConnections = GetPropertyAsBool("Ice.AcceptNonSecureConnections") ?? true;

                {
                    int num = GetPropertyAsInt("Ice.ClassGraphDepthMax") ?? 100;
                    if (num < 1 || num > 0x7fffffff)
                    {
                        ClassGraphDepthMax = 0x7fffffff;
                    }
                    else
                    {
                        ClassGraphDepthMax = num;
                    }
                }

                ToStringMode = Enum.Parse<ToStringMode>(GetProperty("Ice.ToStringMode") ?? "Unicode");

                _backgroundLocatorCacheUpdates = GetPropertyAsBool("Ice.BackgroundLocatorCacheUpdates") ?? false;

                string[]? arr = GetPropertyAsList("Ice.RetryIntervals");

                if (arr == null)
                {
                    _retryIntervals = new int[] { 0 };
                }
                else
                {
                    _retryIntervals = new int[arr.Length];
                    for (int i = 0; i < arr.Length; i++)
                    {
                        int v = int.Parse(arr[i], CultureInfo.InvariantCulture);
                        //
                        // If -1 is the first value, no retry and wait intervals.
                        //
                        if (i == 0 && v == -1)
                        {
                            _retryIntervals = Array.Empty<int>();
                            break;
                        }

                        _retryIntervals[i] = v > 0 ? v : 0;
                    }
                }

                bool isIPv6Supported = Network.IsIPv6Supported();
                bool ipv4 = GetPropertyAsBool("Ice.IPv4") ?? true;
                bool ipv6 = GetPropertyAsBool("Ice.IPv6") ?? isIPv6Supported;
                if (!ipv4 && !ipv6)
                {
                    throw new InvalidConfigurationException("Both IPV4 and IPv6 support cannot be disabled.");
                }
                else if (ipv4 && ipv6)
                {
                    IPVersion = Network.EnableBoth;
                }
                else if (ipv4)
                {
                    IPVersion = Network.EnableIPv4;
                }
                else
                {
                    IPVersion = Network.EnableIPv6;
                }
                PreferIPv6 = GetPropertyAsBool("Ice.PreferIPv6Address") ?? false;

                NetworkProxy = CreateNetworkProxy(IPVersion);

                AddEndpointFactory(new TcpEndpointFactory(
                    new TransportInstance(this, EndpointType.TCP, "tcp", false)));
                AddEndpointFactory(new UdpEndpointFactory(
                    new TransportInstance(this, EndpointType.UDP, "udp", false)));
                AddEndpointFactory(new WSEndpointFactory(
                    new TransportInstance(this, EndpointType.WS, "ws", false), EndpointType.TCP));
                AddEndpointFactory(new WSEndpointFactory(
                    new TransportInstance(this, EndpointType.WSS, "wss", true), EndpointType.SSL));

                _outgoingConnectionFactory = new OutgoingConnectionFactory(this);

                if (GetPropertyAsBool("Ice.PreloadAssemblies") ?? false)
                {
                    AssemblyUtil.PreloadAssemblies();
                }

                //
                // Load plug-ins.
                //
                Debug.Assert(_serverThreadPool == null);
                LoadPlugins(ref args);

                // Initialize the endpoint factories once all the plugins are loaded. This gives the opportunity for the
                // endpoint factories to find underlying factories.
                foreach (IEndpointFactory factory in _typeToEndpointFactory.Values)
                {
                    factory.Initialize();
                }

                //
                // Create Admin facets, if enabled.
                //
                // Note that any logger-dependent admin facet must be created after we load all plugins,
                // since one of these plugins can be a Logger plugin that sets a new logger during loading
                //

                if (GetProperty("Ice.Admin.Enabled") == null)
                {
                    _adminEnabled = GetProperty("Ice.Admin.Endpoints") != null;
                }
                else
                {
                    _adminEnabled = GetPropertyAsBool("Ice.Admin.Enabled") ?? false;
                }

                _adminFacetFilter = new HashSet<string>(
                    (GetPropertyAsList("Ice.Admin.Facets") ?? Array.Empty<string>()).Distinct());

                if (_adminEnabled)
                {
                    //
                    // Process facet
                    //
                    string processFacetName = "Process";
                    if (_adminFacetFilter.Count == 0 || _adminFacetFilter.Contains(processFacetName))
                    {
                        _adminFacets.Add(processFacetName, new Process(this));
                    }

                    //
                    // Logger facet
                    //
                    string loggerFacetName = "Logger";
                    if (_adminFacetFilter.Count == 0 || _adminFacetFilter.Contains(loggerFacetName))
                    {
                        ILoggerAdminLogger loggerAdminLogger = new LoggerAdminLogger(this, Logger);
                        Logger = loggerAdminLogger;
                        _adminFacets.Add(loggerFacetName, loggerAdminLogger.GetFacet());
                    }

                    //
                    // Properties facet
                    //
                    string propertiesFacetName = "Properties";
                    PropertiesAdmin? propsAdmin = null;
                    if (_adminFacetFilter.Count == 0 || _adminFacetFilter.Contains(propertiesFacetName))
                    {
                        propsAdmin = new PropertiesAdmin(this);
                        _adminFacets.Add(propertiesFacetName, propsAdmin);
                    }

                    //
                    // Metrics facet
                    //
                    string metricsFacetName = "Metrics";
                    if (_adminFacetFilter.Count == 0 || _adminFacetFilter.Contains(metricsFacetName))
                    {
                        var communicatorObserver = new CommunicatorObserverI(this, Logger);
                        Observer = communicatorObserver;
                        _adminFacets.Add(metricsFacetName, communicatorObserver.GetFacet());

                        // Make sure the admin plugin receives property updates.
                        if (propsAdmin != null)
                        {
                            propsAdmin.Updated += (_, updates) =>
                                communicatorObserver.GetFacet().Updated(updates);
                        }
                    }
                }

                //
                // Set observer updater
                //
                if (Observer != null)
                {
                    Observer.SetObserverUpdater(new ObserverUpdater(this));
                }

                //
                // Create threads.
                //
                try
                {
                    _timer = new IceInternal.Timer(this, IceInternal.Util.StringToThreadPriority(
                                                   GetProperty("Ice.ThreadPriority")));
                }
                catch (Exception ex)
                {
                    Logger.Error($"cannot create thread for timer:\n{ex}");
                    throw;
                }

                try
                {
                    _endpointHostResolverThread = new HelperThread(this);
                    UpdateEndpointHostResolverObserver();
                    _endpointHostResolverThread.Start(IceInternal.Util.StringToThreadPriority(GetProperty("Ice.ThreadPriority")));
                }
                catch (Exception ex)
                {
                    Logger.Error($"cannot create thread for endpoint host resolver:\n{ex}");
                    throw;
                }
                _clientThreadPool = new IceInternal.ThreadPool(this, "Ice.ThreadPool.Client", 0);

                // The default router/locator may have been set during the loading of plugins.
                // Therefore we only set it if it hasn't already been set.
                try
                {
                    _defaultLocator ??= GetPropertyAsProxy("Ice.Default.Locator", ILocatorPrx.Factory);
                }
                catch (FormatException ex)
                {
                    throw new InvalidConfigurationException("invalid value for Ice.Default.Locator", ex);
                }

                try
                {
                    _defaultRouter ??= GetPropertyAsProxy("Ice.Default.Router", IRouterPrx.Factory);
                }
                catch (FormatException ex)
                {
                    throw new InvalidConfigurationException("invalid value for Ice.Default.Locator", ex);
                }

                //
                // Show process id if requested (but only once).
                //
                lock (this)
                {
                    if (!_printProcessIdDone && (GetPropertyAsBool("Ice.PrintProcessId") ?? false))
                    {
                        using var p = System.Diagnostics.Process.GetCurrentProcess();
                        Console.WriteLine(p.Id);
                        _printProcessIdDone = true;
                    }
                }

                //
                // Server thread pool initialization is lazy in serverThreadPool().
                //

                //
                // An application can set Ice.InitPlugins=0 if it wants to postpone
                // initialization until after it has interacted directly with the
                // plug-ins.
                //
                if (GetPropertyAsBool("Ice.InitPlugins") ?? true)
                {
                    InitializePlugins();
                }

                //
                // This must be done last as this call creates the Ice.Admin object adapter
                // and eventually registers a process proxy with the Ice locator (allowing
                // remote clients to invoke on Ice.Admin facets as soon as it's registered).
                //
                if (!(GetPropertyAsBool("Ice.Admin.DelayCreation") ?? false))
                {
                    GetAdmin();
                }
            }
            catch (Exception)
            {
                Destroy();
                throw;
            }
        }

        public void AddAdminFacet(string facet, IObject servant)
        {
            lock (this)
            {
                if (_state == StateDestroyed)
                {
                    throw new CommunicatorDestroyedException();
                }

                if (_adminFacetFilter.Count > 0 && !_adminFacetFilter.Contains(facet))
                {
                    throw new ArgumentException($"facet `{facet}' not allowed by Ice.Admin.Facets configuration",
                        nameof(facet));
                }

                if (_adminFacets.ContainsKey(facet))
                {
                    throw new ArgumentException($"facet `{facet}' is already registered", nameof(facet));
                }
                _adminFacets.Add(facet, servant);
                if (_adminAdapter != null)
                {
                    Debug.Assert(_adminIdentity != null);
                    _adminAdapter.Add(_adminIdentity.Value, facet, servant);
                }
            }
        }

        /// <summary>
        /// Add the Admin object with all its facets to the provided object adapter.
        /// If Ice.Admin.ServerId is set and the provided object adapter has a Locator,
        /// createAdmin registers the Admin's Process facet with the Locator's LocatorRegistry.
        ///
        /// createAdmin call only be called once; subsequent calls raise InvalidOperationException.
        ///
        /// </summary>
        /// <param name="adminAdapter">The object adapter used to host the Admin object; if null and
        /// Ice.Admin.Endpoints is set, create, activate and use the Ice.Admin object adapter.
        ///
        /// </param>
        /// <param name="adminIdentity">The identity of the Admin object.
        ///
        /// </param>
        /// <returns>A proxy to the main ("") facet of the Admin object. Never returns a null proxy.
        ///
        /// </returns>
        public IObjectPrx CreateAdmin(ObjectAdapter? adminAdapter, Identity adminIdentity)
        {
            lock (this)
            {
                if (_state == StateDestroyed)
                {
                    throw new CommunicatorDestroyedException();
                }

                if (_adminAdapter != null)
                {
                    throw new InvalidOperationException("Admin already created");
                }

                if (!_adminEnabled)
                {
                    throw new InvalidOperationException("Admin is disabled");
                }

                _adminIdentity = adminIdentity;
                if (adminAdapter == null)
                {
                    if (GetProperty("Ice.Admin.Endpoints") != null)
                    {
                        adminAdapter = CreateObjectAdapter("Ice.Admin");
                    }
                    else
                    {
                        throw new InvalidConfigurationException("Ice.Admin.Endpoints is not set");
                    }
                }
                else
                {
                    _adminAdapter = adminAdapter;
                }
                Debug.Assert(_adminAdapter != null);
                AddAllAdminFacets();
            }

            if (adminAdapter == null) // the parameter is null which means _adminAdapter needs to be activated
            {
                try
                {
                    _adminAdapter.Activate();
                }
                catch (Exception)
                {
                    // We cleanup _adminAdapter, however this error is not recoverable
                    // (can't call again getAdmin() after fixing the problem)
                    // since all the facets (servants) in the adapter are lost
                    _adminAdapter.Destroy();
                    lock (this)
                    {
                        _adminAdapter = null;
                    }
                    throw;
                }
            }
            SetServerProcessProxy(_adminAdapter, adminIdentity);
            return _adminAdapter.CreateProxy(adminIdentity, IObjectPrx.Factory);
        }

        /// <summary>
        /// Destroy the communicator.
        /// This operation calls shutdown
        /// implicitly.  Calling destroy cleans up memory, and shuts down
        /// this communicator's client functionality and destroys all object
        /// adapters. Subsequent calls to destroy are ignored.
        /// </summary>
        public void Destroy()
        {
            lock (this)
            {
                //
                // If destroy is in progress, wait for it to be done. This
                // is necessary in case destroy() is called concurrently
                // by multiple threads.
                //
                while (_state == StateDestroyInProgress)
                {
                    Monitor.Wait(this);
                }

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
            Shutdown();
            _outgoingConnectionFactory?.Destroy();

            //
            // First wait for shutdown to finish.
            //
            WaitForShutdown();

            DestroyAllObjectAdapters();

            _outgoingConnectionFactory?.WaitUntilFinished();

            DestroyRetryQueue(); // Must be called before destroying thread pools.

            Observer?.SetObserverUpdater(null);

            if (Logger is ILoggerAdminLogger)
            {
                ((ILoggerAdminLogger)Logger).Destroy();
            }

            //
            // Now, destroy the thread pools. This must be done *only* after
            // all the connections are finished (the connections destruction
            // can require invoking callbacks with the thread pools).
            //
            _serverThreadPool?.Destroy();
            _clientThreadPool?.Destroy();
            _asyncIOThread?.Destroy();

            if (_endpointHostResolverThread != null)
            {
                lock (_endpointHostResolverThread)
                {
                    Debug.Assert(!_endpointHostResolverDestroyed);
                    _endpointHostResolverDestroyed = true;
                    Monitor.Pulse(_endpointHostResolverThread);
                }
            }

            //
            // Wait for all the threads to be finished.
            //
            _timer?.Destroy();
            _clientThreadPool?.JoinWithAllThreads();
            _serverThreadPool?.JoinWithAllThreads();
            _asyncIOThread?.JoinWithThread();

            _endpointHostResolverThread?.Join();

            lock (_routerInfoTable)
            {
                foreach (RouterInfo routerInfo in _routerInfoTable.Values)
                {
                    routerInfo.Destroy();
                }
                _routerInfoTable.Clear();
            }

            lock (_locatorInfoMap)
            {
                foreach (LocatorInfo locatorInfo in _locatorInfoMap.Values)
                {
                    locatorInfo.Destroy();
                }
                _locatorInfoMap.Clear();
                _locatorTableMap.Clear();
            }

            foreach (IEndpointFactory factory in _typeToEndpointFactory.Values)
            {
                factory.Destroy();
            }
            _typeToEndpointFactory.Clear();
            _transportToEndpointFactory.Clear();

            if (GetPropertyAsBool("Ice.Warn.UnusedProperties") ?? false)
            {
                List<string> unusedProperties = GetUnusedProperties();
                if (unusedProperties.Count != 0)
                {
                    var message = new StringBuilder("The following properties were set but never read:");
                    foreach (string s in unusedProperties)
                    {
                        message.Append("\n    ");
                        message.Append(s);
                    }
                    Logger.Warning(message.ToString());
                }
            }

            //
            // Destroy last so that a Logger plugin can receive all log/traces before its destruction.
            //
            List<(string Name, IPlugin Plugin)> plugins;
            lock (this)
            {
                plugins = new List<(string Name, IPlugin Plugin)>(_plugins);
            }
            plugins.Reverse();
            foreach ((string name, IPlugin plugin) in plugins)
            {
                try
                {
                    plugin.Destroy();
                }
                catch (Exception ex)
                {
                    Util.GetProcessLogger().Warning(
                        $"unexpected exception raised by plug-in `{name}' destruction:\n{ex}");
                }
            }

            lock (this)
            {
                _serverThreadPool = null;
                _asyncIOThread = null;

                _adminAdapter = null;
                _adminFacets.Clear();

                _state = StateDestroyed;
                Monitor.PulseAll(this);
            }

            {
                if (Logger != null && Logger is FileLogger)
                {
                    ((FileLogger)Logger).Destroy();
                }
            }
            _currentContext.Dispose();
        }

        public void Dispose() => Destroy();

        /// <summary>
        /// Returns a facet of the Admin object.
        /// </summary>
        /// <param name="facet">The name of the Admin facet.
        /// </param>
        /// <returns>The servant associated with this Admin facet, or
        /// null if no facet is registered with the given name.</returns>
        public IObject? FindAdminFacet(string facet)
        {
            lock (this)
            {
                if (_state == StateDestroyed)
                {
                    throw new CommunicatorDestroyedException();
                }

                if (!_adminFacets.TryGetValue(facet, out IObject? result))
                {
                    return null;
                }
                return result;
            }
        }

        /// <summary>
        /// Returns a map of all facets of the Admin object.
        /// </summary>
        /// <returns>A collection containing all the facet names and
        /// servants of the Admin object.
        ///
        /// </returns>
        public Dictionary<string, IObject> FindAllAdminFacets()
        {
            lock (this)
            {
                if (_state == StateDestroyed)
                {
                    throw new CommunicatorDestroyedException();
                }
                return new Dictionary<string, IObject>(_adminFacets); // TODO, return a read-only collection
            }
        }

        /// <summary>
        /// Get a proxy to the main facet of the Admin object.
        /// GetAdmin also creates the Admin object and creates and activates the Ice.Admin object
        /// adapter to host this Admin object if Ice.Admin.Enpoints is set. The identity of the Admin
        /// object created by getAdmin is {value of Ice.Admin.InstanceName}/admin, or {UUID}/admin
        /// when Ice.Admin.InstanceName is not set.
        ///
        /// If Ice.Admin.DelayCreation is 0 or not set, getAdmin is called by the communicator
        /// initialization, after initialization of all plugins.
        ///
        /// </summary>
        /// <returns>A proxy to the main ("") facet of the Admin object, or a null proxy if no
        /// Admin object is configured.</returns>
        public IObjectPrx? GetAdmin()
        {
            ObjectAdapter adminAdapter;
            Identity adminIdentity;

            lock (this)
            {
                if (_state == StateDestroyed)
                {
                    throw new CommunicatorDestroyedException();
                }

                if (_adminAdapter != null)
                {
                    Debug.Assert(_adminIdentity != null);
                    return _adminAdapter.CreateProxy(_adminIdentity.Value, IObjectPrx.Factory);
                }
                else if (_adminEnabled)
                {
                    if (GetProperty("Ice.Admin.Endpoints") != null)
                    {
                        adminAdapter = CreateObjectAdapter("Ice.Admin");
                    }
                    else
                    {
                        return null;
                    }
                    adminIdentity = new Identity("admin", GetProperty("Ice.Admin.InstanceName") ?? "");
                    if (adminIdentity.Category.Length == 0)
                    {
                        adminIdentity = new Identity(adminIdentity.Name, Guid.NewGuid().ToString());
                    }

                    _adminIdentity = adminIdentity;
                    _adminAdapter = adminAdapter;
                    AddAllAdminFacets();
                    // continue below outside synchronization
                }
                else
                {
                    return null;
                }
            }

            try
            {
                adminAdapter.Activate();
            }
            catch (Exception)
            {
                // We cleanup _adminAdapter, however this error is not recoverable
                // (can't call again getAdmin() after fixing the problem)
                // since all the facets (servants) in the adapter are lost
                adminAdapter.Destroy();
                lock (this)
                {
                    _adminAdapter = null;
                }
                throw;
            }

            SetServerProcessProxy(adminAdapter, adminIdentity);
            return adminAdapter.CreateProxy(adminIdentity, IObjectPrx.Factory);
        }

        /// <summary>
        /// Check whether the communicator has been shut down.
        /// </summary>
        /// <returns>True if the communicator has been shut down; false otherwise.</returns>
        public bool IsShutdown()
        {
            lock (this)
            {
                return _isShutdown;
            }
        }

        /// <summary>Removes an admin facet servant previously added with AddAdminFacet.</summary>
        /// <param name="facet">The Admin facet.</param>
        /// <returns>The admin facet servant that was just removed, or null if the facet was not found.</returns>
        public IObject? RemoveAdminFacet(string facet)
        {
            lock (this)
            {
                if (_adminFacets.TryGetValue(facet, out IObject? result))
                {
                    _adminFacets.Remove(facet);
                }
                else
                {
                    return null;
                }
                if (_adminAdapter != null)
                {
                    Debug.Assert(_adminIdentity != null);
                    _adminAdapter.Remove(_adminIdentity.Value, facet);
                }
                return result;
            }
        }

        public IceInternal.Timer Timer()
        {
            lock (this)
            {
                if (_state == StateDestroyed)
                {
                    throw new CommunicatorDestroyedException();
                }
                return _timer;
            }
        }

        // Registers an endpoint factory.
        // TODO: make public and add Ice prefix when removing ITransportPluginFacade.
        internal void AddEndpointFactory(IEndpointFactory factory)
        {
            _typeToEndpointFactory.Add(factory.Type(), factory);
            _transportToEndpointFactory.Add(factory.Transport(), factory);
        }

        internal AsyncIOThread AsyncIOThread()
        {
            lock (this)
            {
                if (_state == StateDestroyed)
                {
                    throw new CommunicatorDestroyedException();
                }

                if (_asyncIOThread == null) // Lazy initialization.
                {
                    _asyncIOThread = new AsyncIOThread(this);
                }

                return _asyncIOThread;
            }
        }

        internal int CheckRetryAfterException(System.Exception ex, Reference reference, ref int cnt)
        {
            ILogger logger = Logger;

            if (reference.InvocationMode == InvocationMode.BatchOneway ||
                reference.InvocationMode == InvocationMode.BatchDatagram)
            {
                Debug.Assert(false); // batch no longer implemented anyway
                throw ex;
            }

            //
            // If it's a fixed proxy, retrying isn't useful as the proxy is tied to
            // the connection and the request will fail with the exception.
            //
            if (reference.IsFixed)
            {
                throw ex;
            }

            if (ex is ObjectNotExistException one)
            {
                RouterInfo? ri = reference.RouterInfo;
                if (ri != null && one.Operation.Equals("ice_add_proxy"))
                {
                    //
                    // If we have a router, an ObjectNotExistException with an
                    // operation name "ice_add_proxy" indicates to the client
                    // that the router isn't aware of the proxy (for example,
                    // because it was evicted by the router). In this case, we
                    // must *always* retry, so that the missing proxy is added
                    // to the router.
                    //

                    ri.ClearCache(reference);

                    if (TraceLevels.Retry >= 1)
                    {
                        logger.Trace(TraceLevels.RetryCat, $"retrying operation call to add proxy to router\n {ex}");
                    }
                    return 0; // We must always retry, so we don't look at the retry count.
                }
                else if (reference.IsIndirect)
                {
                    //
                    // We retry ObjectNotExistException if the reference is
                    // indirect.
                    //

                    if (reference.IsWellKnown)
                    {
                        reference.LocatorInfo?.ClearCache(reference);
                    }
                }
                else
                {
                    //
                    // For all other cases, we don't retry ObjectNotExistException.
                    //
                    throw ex;
                }
            }

            //
            // Don't retry if the communicator is destroyed, object adapter is deactivated,
            // or connection is manually closed.
            //
            if (ex is CommunicatorDestroyedException ||
                ex is ObjectAdapterDeactivatedException ||
                ex is ConnectionClosedLocallyException)
            {
                throw ex;
            }

            //
            // Don't retry on timeout and operation canceled exceptions.
            //
            if (ex is TimeoutException || ex is OperationCanceledException)
            {
                throw ex;
            }

            ++cnt;
            Debug.Assert(cnt > 0);

            int interval;
            if (cnt == (_retryIntervals.Length + 1) && ex is Ice.ConnectionClosedByPeerException)
            {
                //
                // A connection closed exception is always retried at least once, even if the retry
                // limit is reached.
                //
                interval = 0;
            }
            else if (cnt > _retryIntervals.Length)
            {
                if (TraceLevels.Retry >= 1)
                {
                    string s = "cannot retry operation call because retry limit has been exceeded\n" + ex;
                    logger.Trace(TraceLevels.RetryCat, s);
                }
                throw ex;
            }
            else
            {
                interval = _retryIntervals[cnt - 1];
            }

            if (TraceLevels.Retry >= 1)
            {
                string s = "retrying operation call";
                if (interval > 0)
                {
                    s += " in " + interval + "ms";
                }
                s += " because of exception\n" + ex;
                logger.Trace(TraceLevels.RetryCat, s);
            }

            return interval;
        }

        internal IceInternal.ThreadPool ClientThreadPool()
        {
            lock (this)
            {
                if (_state == StateDestroyed)
                {
                    throw new CommunicatorDestroyedException();
                }
                return _clientThreadPool;
            }
        }

        // Finds an endpoint factory previously registered using AddEndpointFactory.
        internal IEndpointFactory? FindEndpointFactory(string transport) =>
            _transportToEndpointFactory.TryGetValue(transport, out IEndpointFactory? factory) ? factory : null;

         // Finds an endpoint factory previously registered using AddEndpointFactory.
        internal IEndpointFactory? FindEndpointFactory(EndpointType type) =>
            _typeToEndpointFactory.TryGetValue(type, out IEndpointFactory? factory) ? factory : null;

        internal BufSizeWarnInfo GetBufSizeWarn(EndpointType type)
        {
            lock (_setBufSizeWarn)
            {
                BufSizeWarnInfo info;
                if (!_setBufSizeWarn.ContainsKey(type))
                {
                    info = new BufSizeWarnInfo();
                    info.SndWarn = false;
                    info.SndSize = -1;
                    info.RcvWarn = false;
                    info.RcvSize = -1;
                    _setBufSizeWarn.Add(type, info);
                }
                else
                {
                    info = _setBufSizeWarn[type];
                }
                return info;
            }
        }

        internal OutgoingConnectionFactory OutgoingConnectionFactory()
        {
            lock (this)
            {
                if (_state == StateDestroyed)
                {
                    throw new CommunicatorDestroyedException();
                }
                return _outgoingConnectionFactory;
            }
        }

        // Returns the IClassFactory associated with this Slice type ID, not null if not found.
        internal IClassFactory? FindClassFactory(string typeId) =>
            _classFactoryCache.GetOrAdd(typeId, typeId =>
            {
                string className = TypeIdToClassName(typeId);
                foreach (string ns in _classFactoryNamespaces)
                {
                    Type? factoryClass = AssemblyUtil.FindType($"{ns}.{className}");
                    if (factoryClass != null)
                    {
                        return (IClassFactory?)Activator.CreateInstance(factoryClass, false);
                    }
                }
                return null;
            });

        internal IClassFactory? FindClassFactory(int compactId) =>
           _compactIdCache.GetOrAdd(compactId, compactId =>
           {
               foreach (string ns in _classFactoryNamespaces)
               {
                   Type? factoryClass = AssemblyUtil.FindType($"{ns}.CompactId_{compactId}");
                   if (factoryClass != null)
                   {
                       return (IClassFactory?)Activator.CreateInstance(factoryClass, false);
                   }
               }
               return null;
           });

        internal IRemoteExceptionFactory? FindRemoteExceptionFactory(string typeId) =>
            _remoteExceptionFactoryCache.GetOrAdd(typeId, typeId =>
            {
                string className = TypeIdToClassName(typeId);
                foreach (string ns in _remoteExceptionFactoryNamespaces)
                {
                    Type? factoryClass = AssemblyUtil.FindType($"{ns}.{className}");
                    if (factoryClass != null)
                    {
                        return (IRemoteExceptionFactory?)Activator.CreateInstance(factoryClass, false);
                    }
                }
                return null;
            });

        internal IceInternal.ThreadPool ServerThreadPool()
        {
            lock (this)
            {
                if (_state == StateDestroyed)
                {
                    throw new CommunicatorDestroyedException();
                }

                if (_serverThreadPool == null) // Lazy initialization.
                {
                    if (_state == StateDestroyInProgress)
                    {
                        throw new CommunicatorDestroyedException();
                    }
                    _serverThreadPool = new IceInternal.ThreadPool(this, "Ice.ThreadPool.Server",
                        GetPropertyAsInt("Ice.ServerIdleTime") ?? 0);
                }

                return _serverThreadPool;
            }
        }

        internal void SetRcvBufSizeWarn(EndpointType type, int size)
        {
            lock (_setBufSizeWarn)
            {
                BufSizeWarnInfo info = GetBufSizeWarn(type);
                info.RcvWarn = true;
                info.RcvSize = size;
                _setBufSizeWarn[type] = info;
            }
        }

        internal void SetServerProcessProxy(ObjectAdapter adminAdapter, Identity adminIdentity)
        {
            IObjectPrx? admin = adminAdapter.CreateProxy(adminIdentity, IObjectPrx.Factory);
            ILocatorPrx? locator = adminAdapter.Locator;
            string? serverId = GetProperty("Ice.Admin.ServerId");

            if (locator != null && serverId != null)
            {
                IProcessPrx process = admin.Clone(facet: "Process", factory: IProcessPrx.Factory);
                try
                {
                    //
                    // Note that as soon as the process proxy is registered, the communicator might be
                    // shutdown by a remote client and admin facets might start receiving calls.
                    //
                    locator.GetRegistry()!.SetServerProcessProxy(serverId, process);
                }
                catch (Exception ex)
                {
                    if (TraceLevels.Location >= 1)
                    {
                        Logger.Trace(TraceLevels.LocationCat,
                            $"could not register server `{serverId}' with the locator registry:\n{ex}");
                    }
                    throw;
                }

                if (TraceLevels.Location >= 1)
                {
                    Logger.Trace(TraceLevels.LocationCat, $"registered server `{serverId}' with the locator registry");
                }
            }
        }

        internal void SetSndBufSizeWarn(EndpointType type, int size)
        {
            lock (_setBufSizeWarn)
            {
                BufSizeWarnInfo info = GetBufSizeWarn(type);
                info.SndWarn = true;
                info.SndSize = size;
                _setBufSizeWarn[type] = info;
            }
        }

        internal void SetThreadHook(Action threadStart, Action threadStop)
        {
            //
            // No locking, as it can only be called during plug-in loading
            //
            ThreadStart = threadStart;
            ThreadStop = threadStop;
        }

        internal void UpdateConnectionObservers()
        {
            try
            {
                _outgoingConnectionFactory.UpdateConnectionObservers();

                ObjectAdapter[] adapters;
                lock (this)
                {
                    adapters = _adapters.ToArray();
                }

                foreach (ObjectAdapter adapter in adapters)
                {
                    adapter.UpdateConnectionObservers();
                }
            }
            catch (CommunicatorDestroyedException)
            {
            }
        }

        internal void UpdateThreadObservers()
        {
            try
            {
                _clientThreadPool.UpdateObservers();
                if (_serverThreadPool != null)
                {
                    _serverThreadPool.UpdateObservers();
                }

                ObjectAdapter[] adapters;
                lock (this)
                {
                    adapters = _adapters.ToArray();
                }

                foreach (ObjectAdapter adapter in adapters)
                {
                    adapter.UpdateThreadObservers();
                }

                UpdateEndpointHostResolverObserver();

                if (_asyncIOThread != null)
                {
                    _asyncIOThread.UpdateObserver();
                }
                Debug.Assert(Observer != null);
                _timer.UpdateObserver(Observer);
            }
            catch (CommunicatorDestroyedException)
            {
            }
        }

        private static string TypeIdToClassName(string typeId)
        {
            if (!typeId.StartsWith("::", StringComparison.Ordinal))
            {
                throw new InvalidDataException($"`{typeId}' is not a valid Ice type ID");
            }
            return typeId.Substring(2).Replace("::", ".");
        }

        private void AddAllAdminFacets()
        {
            lock (this)
            {
                Debug.Assert(_adminAdapter != null);
                foreach (KeyValuePair<string, IObject> entry in _adminFacets)
                {
                    if (_adminFacetFilter.Count == 0 || _adminFacetFilter.Contains(entry.Key))
                    {
                        Debug.Assert(_adminIdentity != null);
                        _adminAdapter.Add(_adminIdentity.Value, entry.Key, entry.Value);
                    }
                }
            }
        }

        private INetworkProxy? CreateNetworkProxy(int protocolSupport)
        {
            string? proxyHost = GetProperty("Ice.SOCKSProxyHost");
            if (proxyHost != null)
            {
                if (protocolSupport == Network.EnableIPv6)
                {
                    throw new InvalidConfigurationException("IPv6 only is not supported with SOCKS4 proxies");
                }
                return new SOCKSNetworkProxy(proxyHost, GetPropertyAsInt("Ice.SOCKSProxyPort") ?? 1080);
            }

            proxyHost = GetProperty("Ice.HTTPProxyHost");
            if (proxyHost != null)
            {
                return new HTTPNetworkProxy(proxyHost, GetPropertyAsInt("Ice.HTTPProxyPort") ?? 1080);
            }

            return null;
        }
    }
}
