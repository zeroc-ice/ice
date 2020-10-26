// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Collections.Immutable;
using System.Collections.Specialized;
using System.Diagnostics;
using System.Linq;
using System.Net;
using System.Reflection;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice
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

    /// <summary>The central object in Ice. One or more communicators can be instantiated for an Ice application.</summary>
    public sealed partial class Communicator : IDisposable, IAsyncDisposable
    {
        private class ObserverUpdater : Instrumentation.IObserverUpdater
        {
            public ObserverUpdater(Communicator communicator) => _communicator = communicator;

            public void UpdateConnectionObservers() => _communicator.UpdateConnectionObservers();

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

        /// <summary>The connection close timeout.</summary>
        public TimeSpan CloseTimeout { get; }
        /// <summary>The connection establishment timeout.</summary>
        public TimeSpan ConnectTimeout { get; }

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
                catch (ObjectDisposedException)
                {
                    return new Dictionary<string, string>(ImmutableDictionary<string, string>.Empty);
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
                    throw new CommunicatorDisposedException(ex);
                }
            }
        }

        /// <summary>The default context for proxies created using this communicator. Changing the value of
        /// DefaultContext does not change the context of previously created proxies.</summary>
        public IReadOnlyDictionary<string, string> DefaultContext
        {
            get => _defaultContext;
            set => _defaultContext = value.ToImmutableDictionary();
        }

        /// <summary>The default locator for this communicator. To disable the default locator, null can be used.
        /// All newly created proxies and object adapters will use this default locator. Note that setting this property
        /// has no effect on existing proxies or object adapters.</summary>
        public ILocatorPrx? DefaultLocator
        {
            get => _defaultLocator;
            set => _defaultLocator = value;
        }

        /// <summary>Gets whether or not non-secure endpoints should have preference over secure endpoints.
        /// </summary>
        public bool DefaultPreferNonSecure { get; }

        /// <summary>Gets the default source address value used by proxies created with this communicator.</summary>
        public IPAddress? DefaultSourceAddress { get; }

        /// <summary>Gets the default invocation timeout value used by proxies created with this communicator.
        /// </summary>
        public TimeSpan DefaultInvocationTimeout { get; }

        /// <summary>Gets the default value for the locator cache timeout used by proxies created with this
        /// communicator.</summary>
        public TimeSpan DefaultLocatorCacheTimeout { get; }

        /// <summary>The default router for this communicator. To disable the default router, null can be used.
        /// All newly created proxies will use this default router. Note that setting this property has no effect on
        /// existing proxies.</summary>
        public IRouterPrx? DefaultRouter
        {
            get => _defaultRouter;
            set => _defaultRouter = value;
        }

        /// <summary>The logger for this communicator.</summary>
        public ILogger Logger { get; internal set; }

        /// <summary>Gets the communicator observer used by the Ice run-time or null if a communicator observer
        /// was not set during communicator initialization.</summary>
        public Instrumentation.ICommunicatorObserver? Observer { get; }

        /// <summary>The output mode or format for ToString on Ice proxies when the protocol is ice1. See
        /// <see cref="Ice.ToStringMode"/>.</summary>
        public ToStringMode ToStringMode { get; }

        // The communicator's cancellation token is notified of cancellation when the communicator is destroyed.
        internal CancellationToken CancellationToken => _cancellationTokenSource.Token;
        internal int ClassGraphDepthMax { get; }
        internal Acm ClientAcm { get; }
        internal CompressionLevel CompressionLevel { get; }
        internal int CompressionMinSize { get; }

        internal IReadOnlyList<DispatchInterceptor> DispatchInterceptors => _dispatchInterceptors;
        internal int IncomingFrameSizeMax { get; }
        internal IReadOnlyList<InvocationInterceptor> InvocationInterceptors => _invocationInterceptors;
        internal bool IsDisposed => _disposeTask != null;
        internal INetworkProxy? NetworkProxy { get; }

        /// <summary>Gets the maximum number of invocation attempts made to send a request including the original
        /// invocation. It must be a number greater than 0.</summary>
        internal int RetryMaxAttempts { get; }
        internal int RetryBufferSizeMax { get; }
        internal int RetryRequestSizeMax { get; }
        internal Acm ServerAcm { get; }
        internal SlicOptions SlicOptions { get; }
        internal SslEngine SslEngine { get; }
        internal TraceLevels TraceLevels { get; private set; }
        internal bool WarnConnections { get; }
        internal bool WarnDatagrams { get; }
        internal bool WarnDispatch { get; }
        internal bool WarnUnknownProperties { get; }

        private static string[] _emptyArgs = Array.Empty<string>();
        private static readonly string[] _suffixes =
        {
            "EndpointSelection",
            "ConnectionCached",
            "InvocationTimeout",
            "LocatorCacheTimeout",
            "Locator",
            "PreferNonSecure",
            "Router",
            "Context\\..*"
        };
        private static readonly object _staticMutex = new object();

        private readonly HashSet<string> _adapterNamesInUse = new HashSet<string>();
        private readonly List<ObjectAdapter> _adapters = new List<ObjectAdapter>();
        private ObjectAdapter? _adminAdapter;
        private readonly bool _adminEnabled;
        private readonly HashSet<string> _adminFacetFilter = new HashSet<string>();
        private readonly Dictionary<string, IObject> _adminFacets = new Dictionary<string, IObject>();
        private Identity? _adminIdentity;
        private readonly bool _backgroundLocatorCacheUpdates;
        private readonly CancellationTokenSource _cancellationTokenSource = new CancellationTokenSource();
        private readonly ConcurrentDictionary<string, Func<AnyClass>?> _classFactoryCache =
            new ConcurrentDictionary<string, Func<AnyClass>?>();
        private readonly ConcurrentDictionary<int, Func<AnyClass>?> _compactIdCache =
            new ConcurrentDictionary<int, Func<AnyClass>?>();
        private readonly ThreadLocal<Dictionary<string, string>> _currentContext
            = new ThreadLocal<Dictionary<string, string>>();
        private volatile IReadOnlyDictionary<string, string> _defaultContext =
            ImmutableDictionary<string, string>.Empty;
        private volatile ILocatorPrx? _defaultLocator;
        private volatile IRouterPrx? _defaultRouter;
        private readonly List<DispatchInterceptor> _dispatchInterceptors = new List<DispatchInterceptor>();
        private Task? _disposeTask;
        private readonly List<InvocationInterceptor> _invocationInterceptors = new List<InvocationInterceptor>();
        private static readonly Dictionary<string, Assembly> _loadedAssemblies = new Dictionary<string, Assembly>();
        private readonly ConcurrentDictionary<ILocatorPrx, LocatorInfo> _locatorInfoMap =
            new ConcurrentDictionary<ILocatorPrx, LocatorInfo>();

        private readonly object _mutex = new object();
        private static bool _oneOffDone;
        private static bool _printProcessIdDone;
        private readonly ConcurrentDictionary<
            string, Func<string?, RemoteExceptionOrigin?, RemoteException>?> _remoteExceptionFactoryCache = new ();
        private int _retryBufferSize;
        private readonly ConcurrentDictionary<IRouterPrx, RouterInfo> _routerInfoTable =
            new ConcurrentDictionary<IRouterPrx, RouterInfo>();
        private readonly Dictionary<Transport, BufSizeWarnInfo> _setBufSizeWarn =
            new Dictionary<Transport, BufSizeWarnInfo>();
        private Task? _shutdownTask;
        private TaskCompletionSource<object?>? _waitForShutdownCompletionSource;

        private readonly IDictionary<Transport, Ice1EndpointFactory> _ice1TransportRegistry =
            new ConcurrentDictionary<Transport, Ice1EndpointFactory>();

        private readonly IDictionary<Transport, (Ice2EndpointFactory, Ice2EndpointParser)> _ice2TransportRegistry =
            new ConcurrentDictionary<Transport, (Ice2EndpointFactory, Ice2EndpointParser)>();

        private readonly IDictionary<string, (Ice1EndpointParser, Transport)> _ice1TransportNameRegistry =
            new ConcurrentDictionary<string, (Ice1EndpointParser, Transport)>();

        private readonly IDictionary<string, (Ice2EndpointParser, Transport)> _ice2TransportNameRegistry =
            new ConcurrentDictionary<string, (Ice2EndpointParser, Transport)>();

        /// <summary>Constructs a new communicator.</summary>
        /// <param name="properties">The properties of the new communicator.</param>
        /// <param name="dispatchInterceptors">A collection of <see cref="DispatchInterceptor"/> that will be
        /// executed with each dispatch.</param>
        /// <param name="invocationInterceptors">A collection of <see cref="InvocationInterceptor"/> that will
        /// be executed with each invocation.</param>
        /// <param name="logger">The logger used by the new communicator.</param>
        /// <param name="observer">The communicator observer used by the new communicator.</param>
        /// <param name="tlsClientOptions">Client side configuration for TLS connections.</param>
        /// <param name="tlsServerOptions">Server side configuration for TLS connections.</param>
        public Communicator(
            IReadOnlyDictionary<string, string> properties,
            IEnumerable<DispatchInterceptor>? dispatchInterceptors = null,
            IEnumerable<InvocationInterceptor>? invocationInterceptors = null,
            ILogger? logger = null,
            Instrumentation.ICommunicatorObserver? observer = null,
            TlsClientOptions? tlsClientOptions = null,
            TlsServerOptions? tlsServerOptions = null)
            : this(ref _emptyArgs,
                   appSettings: null,
                   dispatchInterceptors,
                   invocationInterceptors,
                   logger,
                   observer,
                   properties,
                   tlsClientOptions,
                   tlsServerOptions)
        {
        }

        /// <summary>Constructs a new communicator.</summary>
        /// <param name="args">An array of command-line arguments used to set or override Ice.* properties.</param>
        /// <param name="properties">The properties of the new communicator.</param>
        /// <param name="dispatchInterceptors">A collection of <see cref="DispatchInterceptor"/> that will be
        /// executed with each dispatch.</param>
        /// <param name="invocationInterceptors">A collection of <see cref="InvocationInterceptor"/> that will
        /// be executed with each invocation.</param>
        /// <param name="logger">The logger used by the new communicator.</param>
        /// <param name="observer">The communicator observer used by the new communicator.</param>
        /// <param name="tlsClientOptions">Client side configuration for TLS connections.</param>
        /// <param name="tlsServerOptions">Server side configuration for TLS connections.</param>
        public Communicator(
            ref string[] args,
            IReadOnlyDictionary<string, string> properties,
            IEnumerable<DispatchInterceptor>? dispatchInterceptors = null,
            IEnumerable<InvocationInterceptor>? invocationInterceptors = null,
            ILogger? logger = null,
            Instrumentation.ICommunicatorObserver? observer = null,
            TlsClientOptions? tlsClientOptions = null,
            TlsServerOptions? tlsServerOptions = null)
            : this(ref args,
                   appSettings: null,
                   dispatchInterceptors,
                   invocationInterceptors,
                   logger,
                   observer,
                   properties,
                   tlsClientOptions,
                   tlsServerOptions)
        {
        }

        /// <summary>Constructs a new communicator.</summary>
        /// <param name="appSettings">Collection of settings to configure the new communicator properties. The appSettings
        /// param has precedence over the properties param.</param>
        /// <param name="dispatchInterceptors">A collection of <see cref="DispatchInterceptor"/> that will be
        /// executed with each dispatch.</param>
        /// <param name="invocationInterceptors">A collection of <see cref="InvocationInterceptor"/> that will
        /// be executed with each invocation.</param>
        /// <param name="logger">The logger used by the new communicator.</param>
        /// <param name="observer">The communicator observer used by the Ice run-time.</param>
        /// <param name="properties">The properties of the new communicator.</param>
        /// <param name="tlsClientOptions">Client side configuration for TLS connections.</param>
        /// <param name="tlsServerOptions">Server side configuration for TLS connections.</param>
        public Communicator(
            NameValueCollection? appSettings = null,
            IEnumerable<DispatchInterceptor>? dispatchInterceptors = null,
            IEnumerable<InvocationInterceptor>? invocationInterceptors = null,
            ILogger? logger = null,
            Instrumentation.ICommunicatorObserver? observer = null,
            IReadOnlyDictionary<string, string>? properties = null,
            TlsClientOptions? tlsClientOptions = null,
            TlsServerOptions? tlsServerOptions = null)
            : this(ref _emptyArgs,
                   appSettings,
                   dispatchInterceptors,
                   invocationInterceptors,
                   logger,
                   observer,
                   properties,
                   tlsClientOptions,
                   tlsServerOptions)
        {
        }

        /// <summary>Constructs a new communicator.</summary>
        /// <param name="args">An array of command-line arguments used to set or override Ice.* properties.</param>
        /// <param name="appSettings">Collection of settings to configure the new communicator properties. The appSettings
        /// param has precedence over the properties param.</param>
        /// <param name="dispatchInterceptors">A collection of <see cref="DispatchInterceptor"/> that will be
        /// executed with each dispatch.</param>
        /// <param name="invocationInterceptors">A collection of <see cref="InvocationInterceptor"/> that will
        /// be executed with each invocation.</param>
        /// <param name="logger">The logger used by the new communicator.</param>
        /// <param name="observer">The communicator observer used by the new communicator.</param>
        /// <param name="properties">The properties of the new communicator.</param>
        /// <param name="tlsClientOptions">Client side configuration for TLS connections.</param>
        /// <param name="tlsServerOptions">Server side configuration for TLS connections.</param>
        public Communicator(
            ref string[] args,
            NameValueCollection? appSettings = null,
            IEnumerable<DispatchInterceptor>? dispatchInterceptors = null,
            IEnumerable<InvocationInterceptor>? invocationInterceptors = null,
            ILogger? logger = null,
            Instrumentation.ICommunicatorObserver? observer = null,
            IReadOnlyDictionary<string, string>? properties = null,
            TlsClientOptions? tlsClientOptions = null,
            TlsServerOptions? tlsServerOptions = null)
        {
            Logger = logger ?? Runtime.Logger;
            Observer = observer;

            // clone properties as we don't want to modify the properties given to this constructor
            var combinedProperties =
                new Dictionary<string, string>(properties ?? ImmutableDictionary<string, string>.Empty);

            if (appSettings != null)
            {
                foreach (string? key in appSettings.AllKeys)
                {
                    if (key != null)
                    {
                        string[]? values = appSettings.GetValues(key);
                        if (values == null)
                        {
                            combinedProperties[key] = "";
                        }
                        else if (values.Length == 1)
                        {
                            combinedProperties[key] = values[0];
                        }
                        else
                        {
                            combinedProperties[key] = StringUtil.ToPropertyValue(values);
                        }
                    }
                }
            }

            if (!combinedProperties.ContainsKey("Ice.ProgramName"))
            {
                combinedProperties["Ice.ProgramName"] = AppDomain.CurrentDomain.FriendlyName;
            }

            combinedProperties.ParseIceArgs(ref args);
            SetProperties(combinedProperties);

            try
            {
                lock (_staticMutex)
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
                            if (stdErr == stdOut)
                            {
                                Debug.Assert(outStream != null);
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

                        UriParser.RegisterCommon();
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
                    else if (Runtime.Logger is Logger)
                    {
                        // Ice.ConsoleListener is enabled by default.
                        Logger = new TraceLogger(programName, GetPropertyAsBool("Ice.ConsoleListener") ?? true);
                    }
                    // else already set to process logger
                }

                TraceLevels = new TraceLevels(this);

                DefaultInvocationTimeout =
                    GetPropertyAsTimeSpan("Ice.Default.InvocationTimeout") ?? TimeSpan.FromSeconds(60);
                if (DefaultInvocationTimeout == TimeSpan.Zero)
                {
                    throw new InvalidConfigurationException("0 is not a valid value for Ice.Default.InvocationTimeout");
                }

                // TODO: switch to 0/false default
                DefaultPreferNonSecure = GetPropertyAsBool("Ice.Default.PreferNonSecure") ?? true;

                if (GetProperty("Ice.Default.SourceAddress") is string address)
                {
                    try
                    {
                        DefaultSourceAddress = IPAddress.Parse(address);
                    }
                    catch (FormatException ex)
                    {
                        throw new InvalidConfigurationException(
                            $"invalid IP address set for Ice.Default.SourceAddress: `{address}'", ex);
                    }
                }

                // For locator cache timeout, 0 means disable locator cache.
                DefaultLocatorCacheTimeout =
                    GetPropertyAsTimeSpan("Ice.Default.LocatorCacheTimeout") ?? Timeout.InfiniteTimeSpan;

                CloseTimeout = GetPropertyAsTimeSpan("Ice.CloseTimeout") ?? TimeSpan.FromSeconds(10);
                if (CloseTimeout == TimeSpan.Zero)
                {
                    throw new InvalidConfigurationException("0 is not a valid value for Ice.CloseTimeout");
                }

                ConnectTimeout = GetPropertyAsTimeSpan("Ice.ConnectTimeout") ?? TimeSpan.FromSeconds(10);
                if (ConnectTimeout == TimeSpan.Zero)
                {
                    throw new InvalidConfigurationException("0 is not a valid value for Ice.ConnectTimeout");
                }

                ClientAcm = new Acm(this, "Ice.ACM.Client", new Acm(this, "Ice.ACM", Acm.ClientDefault));
                ServerAcm = new Acm(this, "Ice.ACM.Server", new Acm(this, "Ice.ACM", Acm.ServerDefault));

                SlicOptions = new SlicOptions
                {
                    MaxBidirectionalStreams = GetPropertyAsInt("Ice.Slic.MaxBidirectionalStreams") ?? 100,
                    MaxUnidirectionalStreams = GetPropertyAsInt("Ice.Slic.MaxBidirectionalStreams") ?? 100,
                    IdleTimeout = GetPropertyAsTimeSpan("Ice.Slic.IdleTimeout") ?? TimeSpan.FromSeconds(30),
                    PacketSize = GetPropertyAsInt("Ice.Slic.PacketSize") ?? 32 * 1024
                };
                if (SlicOptions.MaxBidirectionalStreams < 1)
                {
                    throw new InvalidConfigurationException($"{SlicOptions.MaxBidirectionalStreams} is not a valid " +
                        "value for Ice.Slic.MaxBidirectionalStreams");
                }
                if (SlicOptions.MaxUnidirectionalStreams < 1)
                {
                    throw new InvalidConfigurationException($"{SlicOptions.MaxBidirectionalStreams} is not a valid " +
                        "value for Ice.Slic.MaxUnidirectionalStreams");
                }
                if (SlicOptions.PacketSize < 1024)
                {
                    throw new InvalidConfigurationException($"{SlicOptions.PacketSize} is not a valid " +
                        "value for Ice.Slic.PacketSize");
                }

                int frameSizeMax = GetPropertyAsByteSize("Ice.IncomingFrameSizeMax") ?? 1024 * 1024;
                IncomingFrameSizeMax = frameSizeMax == 0 ? int.MaxValue : frameSizeMax;

                RetryMaxAttempts = GetPropertyAsInt("Ice.RetryMaxAttempts") ?? 5;

                if (RetryMaxAttempts <= 0)
                {
                    throw new InvalidConfigurationException($"Ice.RetryMaxAttempts must be greater than 0");
                }
                RetryMaxAttempts = Math.Min(RetryMaxAttempts, 5);
                RetryBufferSizeMax = GetPropertyAsByteSize("Ice.RetryBufferSizeMax") ?? 1024 * 1024 * 100;
                RetryRequestSizeMax = GetPropertyAsByteSize("Ice.RetryRequestSizeMax") ?? 1024 * 1024;

                WarnConnections = GetPropertyAsBool("Ice.Warn.Connections") ?? false;
                WarnDatagrams = GetPropertyAsBool("Ice.Warn.Datagrams") ?? false;
                WarnDispatch = GetPropertyAsBool("Ice.Warn.Dispatch") ?? false;
                WarnUnknownProperties = GetPropertyAsBool("Ice.Warn.UnknownProperties") ?? true;

                CompressionLevel =
                    GetPropertyAsEnum<CompressionLevel>("Ice.CompressionLevel") ?? CompressionLevel.Fastest;
                CompressionMinSize = GetPropertyAsByteSize("Ice.CompressionMinSize") ?? 100;

                // TODO: switch to 0 default
                AcceptNonSecureConnections = GetPropertyAsBool("Ice.AcceptNonSecureConnections") ?? true;
                int classGraphDepthMax = GetPropertyAsInt("Ice.ClassGraphDepthMax") ?? 100;
                ClassGraphDepthMax = classGraphDepthMax < 1 ? int.MaxValue : classGraphDepthMax;

                ToStringMode = GetPropertyAsEnum<ToStringMode>("Ice.ToStringMode") ?? default;

                _backgroundLocatorCacheUpdates = GetPropertyAsBool("Ice.BackgroundLocatorCacheUpdates") ?? false;

                NetworkProxy = CreateNetworkProxy(Network.EnableBoth);

                SslEngine = new SslEngine(this, tlsClientOptions, tlsServerOptions);

                RegisterIce1Transport(Transport.TCP,
                                      "tcp",
                                      TcpEndpoint.CreateIce1Endpoint,
                                      TcpEndpoint.ParseIce1Endpoint);

                RegisterIce1Transport(Transport.SSL,
                                      "ssl",
                                      TcpEndpoint.CreateIce1Endpoint,
                                      TcpEndpoint.ParseIce1Endpoint);

                RegisterIce1Transport(Transport.UDP,
                                      "udp",
                                      UdpEndpoint.CreateIce1Endpoint,
                                      UdpEndpoint.ParseIce1Endpoint);

                RegisterIce1Transport(Transport.WS,
                                      "ws",
                                      WSEndpoint.CreateIce1Endpoint,
                                      WSEndpoint.ParseIce1Endpoint);

                RegisterIce1Transport(Transport.WSS,
                                      "wss",
                                      WSEndpoint.CreateIce1Endpoint,
                                      WSEndpoint.ParseIce1Endpoint);

                RegisterIce2Transport(Transport.TCP,
                                      "tcp",
                                      TcpEndpoint.CreateIce2Endpoint,
                                      TcpEndpoint.ParseIce2Endpoint,
                                      IPEndpoint.DefaultIPPort);

                RegisterIce2Transport(Transport.WS,
                                      "ws",
                                      WSEndpoint.CreateIce2Endpoint,
                                      WSEndpoint.ParseIce2Endpoint,
                                      IPEndpoint.DefaultIPPort);

                // TODO: this is temporary, ice2 should not have ssl and wss transports
                RegisterIce2Transport(Transport.SSL,
                                      "ssl",
                                      TcpEndpoint.CreateIce2Endpoint,
                                      TcpEndpoint.ParseIce2Endpoint,
                                      IPEndpoint.DefaultIPPort);

                RegisterIce2Transport(Transport.WSS,
                                      "wss",
                                      WSEndpoint.CreateIce2Endpoint,
                                      WSEndpoint.ParseIce2Endpoint,
                                      IPEndpoint.DefaultIPPort);

                OutgoingConnectionFactory = new OutgoingConnectionFactory(this);

                if (GetPropertyAsBool("Ice.PreloadAssemblies") ?? false)
                {
                    LoadAssemblies();
                }

                if (dispatchInterceptors != null)
                {
                    _dispatchInterceptors.AddRange(dispatchInterceptors);
                }

                if (invocationInterceptors != null)
                {
                    _invocationInterceptors.AddRange(invocationInterceptors);
                }

                // Load plug-ins.
                LoadPlugins(ref args);

                // Create Admin facets, if enabled.
                //
                // Note that any logger-dependent admin facet must be created after we load all plugins,
                // since one of these plugins can be a Logger plugin that sets a new logger during loading

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
                    // Process facet
                    string processFacetName = "Process";
                    if (_adminFacetFilter.Count == 0 || _adminFacetFilter.Contains(processFacetName))
                    {
                        _adminFacets.Add(processFacetName, new Process(this));
                    }

                    // Logger facet
                    string loggerFacetName = "Logger";
                    if (_adminFacetFilter.Count == 0 || _adminFacetFilter.Contains(loggerFacetName))
                    {
                        var loggerAdminLogger = new LoggerAdminLogger(this, Logger);
                        Logger = loggerAdminLogger;
                        _adminFacets.Add(loggerFacetName, loggerAdminLogger.GetFacet());
                    }

                    // Properties facet
                    string propertiesFacetName = "Properties";
                    PropertiesAdmin? propsAdmin = null;
                    if (_adminFacetFilter.Count == 0 || _adminFacetFilter.Contains(propertiesFacetName))
                    {
                        propsAdmin = new PropertiesAdmin(this);
                        _adminFacets.Add(propertiesFacetName, propsAdmin);
                    }

                    // Metrics facet
                    string metricsFacetName = "Metrics";
                    if (_adminFacetFilter.Count == 0 || _adminFacetFilter.Contains(metricsFacetName))
                    {
                        var communicatorObserver = new CommunicatorObserver(this, Logger);
                        Observer = communicatorObserver;
                        _adminFacets.Add(metricsFacetName, communicatorObserver.AdminFacet);

                        // Make sure the admin plugin receives property updates.
                        if (propsAdmin != null)
                        {
                            propsAdmin.Updated += (_, updates) => communicatorObserver.AdminFacet.Updated(updates);
                        }
                    }
                }

                Observer?.SetObserverUpdater(new ObserverUpdater(this));

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

                // Show process id if requested (but only once).
                lock (_mutex)
                {
                    if (!_printProcessIdDone && (GetPropertyAsBool("Ice.PrintProcessId") ?? false))
                    {
                        using var p = System.Diagnostics.Process.GetCurrentProcess();
                        Console.WriteLine(p.Id);
                        _printProcessIdDone = true;
                    }
                }

                // An application can set Ice.InitPlugins=0 if it wants to postpone initialization until after it has
                // interacted directly with the plug-ins.
                if (GetPropertyAsBool("Ice.InitPlugins") ?? true)
                {
                    InitializePlugins();
                }

                // This must be done last as this call creates the Ice.Admin object adapter and eventually registers a
                // process proxy with the Ice locator (allowing remote clients to invoke on Ice.Admin facets as soon as
                // it's registered).
                if (!(GetPropertyAsBool("Ice.Admin.DelayCreation") ?? false))
                {
                    GetAdmin();
                }
            }
            catch
            {
                Dispose();
                throw;
            }
        }

        /// <summary>Adds an admin facet to the communicator.</summary>
        /// <param name="facet">The facet name.</param>
        /// <param name="servant">The servant that implements the admin facet.</param>
        /// <exception cref="ArgumentException">If the configuration doesn't allow adding a facet with
        /// the given name or if there is already a facet with the given name registered with the communicator.
        /// </exception>
        public void AddAdminFacet(string facet, IObject servant)
        {
            lock (_mutex)
            {
                if (IsDisposed)
                {
                    throw new CommunicatorDisposedException();
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

        /// <summary>Adds the Admin object with all its facets to the provided object adapter. If Ice.Admin.ServerId is
        /// set and the provided object adapter has a Locator, createAdmin registers the Admin's Process facet with the
        /// Locator's LocatorRegistry. CreateAdmin can only be called once; subsequent calls raise
        /// InvalidOperationException.</summary>
        /// <param name="adminAdapter">The object adapter used to host the Admin object; if null and
        /// Ice.Admin.Endpoints is set, create, activate and use the Ice.Admin object adapter.</param>
        /// <param name="adminIdentity">The identity of the Admin object.</param>
        /// <returns>A proxy to the Admin object.</returns>
        public IObjectPrx CreateAdmin(ObjectAdapter? adminAdapter, Identity adminIdentity)
        {
            try
            {
                ValueTask<IObjectPrx> task = CreateAdminAsync(adminAdapter, adminIdentity);
                return task.IsCompleted ? task.Result : task.AsTask().Result;
            }
            catch (AggregateException ex)
            {
                Debug.Assert(ex.InnerException != null);
                throw ExceptionUtil.Throw(ex.InnerException);
            }
        }

        /// <summary>Add the Admin object with all its facets to the provided object adapter. If Ice.Admin.ServerId is
        /// set and the provided object adapter has a Locator, createAdmin registers the Admin's Process facet with the
        /// Locator's LocatorRegistry. CreateAdmin can only be called once; subsequent calls raise
        /// InvalidOperationException.</summary>
        /// <param name="adminAdapter">The object adapter used to host the Admin object; if null and
        /// Ice.Admin.Endpoints is set, create, activate and use the Ice.Admin object adapter.</param>
        /// <param name="adminIdentity">The identity of the Admin object.</param>
        /// <returns>A proxy to the Admin object.</returns>
        public async ValueTask<IObjectPrx> CreateAdminAsync(ObjectAdapter? adminAdapter, Identity adminIdentity)
        {
            lock (_mutex)
            {
                if (IsDisposed)
                {
                    throw new CommunicatorDisposedException();
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
                    await _adminAdapter.ActivateAsync().ConfigureAwait(false);
                }
                catch
                {
                    // We cleanup _adminAdapter, however this error is not recoverable (can't call again GetAdmin()
                    // after fixing the problem) since all the facets (servants) in the adapter are lost
                    await _adminAdapter.DisposeAsync().ConfigureAwait(false);
                    lock (_mutex)
                    {
                        _adminAdapter = null;
                    }
                    throw;
                }
            }
            SetServerProcessProxy(_adminAdapter, adminIdentity);
            return _adminAdapter.CreateProxy(adminIdentity, IObjectPrx.Factory);
        }

        /// <summary>Releases resources used by the communicator. This operation calls <see cref="ShutdownAsync"/>
        /// implicitly.</summary>
        public async ValueTask DisposeAsync()
        {
            // If Dispose is in progress just await the _disposeTask, otherwise call PerformDisposeAsync and then await
            // the _disposeTask.
            lock (_mutex)
            {
                _disposeTask ??= PerformDisposeAsync();
            }
            await _disposeTask.ConfigureAwait(false);

            async Task PerformDisposeAsync()
            {
                // Cancel operations that are waiting and using the communicator's cancellation token
                _cancellationTokenSource.Cancel();

                // Shutdown and destroy all the incoming and outgoing Ice connections and wait for the connections to be
                // finished.
                await Task.WhenAll(OutgoingConnectionFactory?.DisposeAsync().AsTask() ?? Task.CompletedTask,
                                   ShutdownAsync()).ConfigureAwait(false);

                // _adminAdapter is disposed by ShutdownAsync call above when iterating over all adapters, we call
                // DisposeAsync here to avoid the compiler warning about disposable field not being dispose.
                if (_adminAdapter != null)
                {
                    await _adminAdapter.DisposeAsync().ConfigureAwait(false);
                }

                Observer?.SetObserverUpdater(null);

                if (Logger is LoggerAdminLogger adminLogger)
                {
                    await adminLogger.DisposeAsync().ConfigureAwait(false);
                }

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

                // Destroy last so that a Logger plugin can receive all log/traces before its destruction.
                foreach ((string name, IPlugin plugin) in Enumerable.Reverse(_plugins))
                {
                    try
                    {
                        await plugin.DisposeAsync().ConfigureAwait(false);
                    }
                    catch (Exception ex)
                    {
                        Runtime.Logger.Warning($"unexpected exception raised by plug-in `{name}' destruction:\n{ex}");
                    }
                }

                if (Logger is FileLogger fileLogger)
                {
                    fileLogger.Dispose();
                }
                _currentContext.Dispose();
                _cancellationTokenSource.Dispose();
            }
        }

        /// <summary>Releases resources used by the communicator. This operation calls <see cref="ShutdownAsync"/>
        /// implicitly.</summary>
        public void Dispose() => DisposeAsync().AsTask().Wait();

        /// <summary>Returns a facet of the Admin object.</summary>
        /// <param name="facet">The name of the Admin facet.</param>
        /// <returns>The servant associated with this Admin facet, or null if no facet is registered with the given
        /// name.</returns>
        public IObject? FindAdminFacet(string facet)
        {
            lock (_mutex)
            {
                if (IsDisposed)
                {
                    throw new CommunicatorDisposedException();
                }

                if (!_adminFacets.TryGetValue(facet, out IObject? result))
                {
                    return null;
                }
                return result;
            }
        }

        /// <summary>Returns a map of all facets of the Admin object.</summary>
        /// <returns>A collection containing all the facet names and servants of the Admin object.</returns>
        public IReadOnlyDictionary<string, IObject> FindAllAdminFacets()
        {
            lock (_mutex)
            {
                if (IsDisposed)
                {
                    throw new CommunicatorDisposedException();
                }
                return _adminFacets.ToImmutableDictionary();
            }
        }

        /// <summary>Get a proxy to the main facet of the Admin object. GetAdmin also creates the Admin object and
        /// creates and activates the Ice.Admin object adapter to host this Admin object if Ice.Admin.Endpoints is set.
        /// The identity of the Admin object created by getAdmin is {value of Ice.Admin.InstanceName}/admin, or
        /// {UUID}/admin when Ice.Admin.InstanceName is not set.
        ///
        /// If Ice.Admin.DelayCreation is 0 or not set, GetAdmin is called by the communicator initialization, after
        /// initialization of all plugins.</summary>
        /// <returns>A proxy to the Admin object, or a null proxy if no Admin object is configured.</returns>
        public IObjectPrx? GetAdmin()
        {
            try
            {
                ValueTask<IObjectPrx?> task = GetAdminAsync();
                return task.IsCompleted ? task.Result : task.AsTask().Result;
            }
            catch (AggregateException ex)
            {
                Debug.Assert(ex.InnerException != null);
                throw ExceptionUtil.Throw(ex.InnerException);
            }
        }

        /// <summary>Get a proxy to the main facet of the Admin object. GetAdminAsync also creates the Admin object and
        /// creates and activates the Ice.Admin object adapter to host this Admin object if Ice.Admin.Endpoints is set.
        /// The identity of the Admin object created by getAdmin is {value of Ice.Admin.InstanceName}/admin, or
        /// {UUID}/admin when Ice.Admin.InstanceName is not set.
        ///
        /// If Ice.Admin.DelayCreation is 0 or not set, GetAdminAsync is called by the communicator initialization,
        /// after initialization of all plugins.</summary>
        /// <returns>A proxy to the Admin object, or a null proxy if no Admin object is configured.</returns>
        public async ValueTask<IObjectPrx?> GetAdminAsync()
        {
            ObjectAdapter adminAdapter;
            Identity adminIdentity;

            lock (_mutex)
            {
                if (IsDisposed)
                {
                    throw new CommunicatorDisposedException();
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
                await adminAdapter.ActivateAsync().ConfigureAwait(false);
            }
            catch
            {
                // We cleanup _adminAdapter, however this error is not recoverable (can't call again getAdmin() after
                // fixing the problem) since all the facets (servants) in the adapter are lost
                await adminAdapter.DisposeAsync().ConfigureAwait(false);
                lock (_mutex)
                {
                    _adminAdapter = null;
                }
                throw;
            }

            SetServerProcessProxy(adminAdapter, adminIdentity);
            return adminAdapter.CreateProxy(adminIdentity, IObjectPrx.Factory);
        }

        /// <summary>Removes an admin facet servant previously added with AddAdminFacet.</summary>
        /// <param name="facet">The Admin facet.</param>
        /// <returns>The admin facet servant that was just removed, or null if the facet was not found.</returns>
        public IObject? RemoveAdminFacet(string facet)
        {
            lock (_mutex)
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

        /// <summary>Registers a new transport for the ice1 protocol.</summary>
        /// <param name="transport">The transport.</param>
        /// <param name="transportName">The name of the transport in lower case, for example "tcp".</param>
        /// <param name="factory">A delegate that Ice will use to unmarshal endpoints for this transport.</param>
        /// <param name="parser">A delegate that Ice will use to parse endpoints for this transport.</param>
        public void RegisterIce1Transport(
            Transport transport,
            string transportName,
            Ice1EndpointFactory factory,
            Ice1EndpointParser parser)
        {
            if (transportName.Length == 0)
            {
                throw new ArgumentException($"{nameof(transportName)} cannot be empty", nameof(transportName));
            }

            _ice1TransportRegistry.Add(transport, factory);
            _ice1TransportNameRegistry.Add(transportName, (parser, transport));
        }

        /// <summary>Registers a new transport for the ice2 protocol.</summary>
        /// <param name="transport">The transport.</param>
        /// <param name="transportName">The name of the transport in lower case, for example "tcp".</param>
        /// <param name="factory">A delegate that Ice will use to unmarshal endpoints for this transport.</param>
        /// <param name="parser">A delegate that Ice will use to parse endpoints for this transport.</param>
        /// <param name="defaultPort">The default port for URI endpoints that don't specificy a port explicitly.</param>
        public void RegisterIce2Transport(
            Transport transport,
            string transportName,
            Ice2EndpointFactory factory,
            Ice2EndpointParser parser,
            ushort defaultPort)
        {
            if (transportName.Length == 0)
            {
                throw new ArgumentException($"{nameof(transportName)} cannot be empty", nameof(transportName));
            }

            _ice2TransportRegistry.Add(transport, (factory, parser));
            _ice2TransportNameRegistry.Add(transportName, (parser, transport));

            // Also register URI parser if not registered yet.
            try
            {
                UriParser.RegisterTransport(transportName, defaultPort);
            }
            catch (InvalidOperationException)
            {
                // Ignored, already registered
            }
        }

        // Add one or more dispatch interceptors, only to use by PluginInitializationContext
        internal void AddDispatchInterceptor(DispatchInterceptor[] interceptors) =>
            _dispatchInterceptors.AddRange(interceptors);

        // Add one or more invocation interceptors, only to use by PluginInitializationContext
        internal void AddInvocationInterceptor(InvocationInterceptor[] interceptors) =>
            _invocationInterceptors.AddRange(interceptors);

        internal void DecRetryBufferSize(int size)
        {
            lock (_mutex)
            {
                Debug.Assert(size <= _retryBufferSize);
                _retryBufferSize -= size;
            }
        }

        internal Ice1EndpointFactory? FindIce1EndpointFactory(Transport transport) =>
            _ice1TransportRegistry.TryGetValue(transport, out Ice1EndpointFactory? factory) ? factory : null;

        internal (Ice1EndpointParser Parser, Transport Transport)? FindIce1EndpointParser(string transportName) =>
            _ice1TransportNameRegistry.TryGetValue(
                transportName,
                out (Ice1EndpointParser, Transport) value) ? value : null;

        internal Ice2EndpointFactory? FindIce2EndpointFactory(Transport transport) =>
            _ice2TransportRegistry.TryGetValue(
                transport,
                out (Ice2EndpointFactory Factory, Ice2EndpointParser _) value) ? value.Factory : null;

        internal Ice2EndpointParser? FindIce2EndpointParser(Transport transport) =>
            _ice2TransportRegistry.TryGetValue(
                transport,
                out (Ice2EndpointFactory _, Ice2EndpointParser Parser) value) ? value.Parser : null;

        internal (Ice2EndpointParser Parser, Transport Transport)? FindIce2EndpointParser(string transportName) =>
            _ice2TransportNameRegistry.TryGetValue(
                transportName,
                out (Ice2EndpointParser, Transport) value) ? value : null;

        internal void EraseRouterInfo(IRouterPrx? router)
        {
            // Removes router info for a given router.
            if (router != null)
            {
                // The router cannot be routed.
                _routerInfoTable.TryRemove(router.Clone(clearRouter: true), out RouterInfo? _);
            }
        }

        internal BufSizeWarnInfo GetBufSizeWarn(Transport transport)
        {
            lock (_setBufSizeWarn)
            {
                BufSizeWarnInfo info;
                if (!_setBufSizeWarn.ContainsKey(transport))
                {
                    info = new BufSizeWarnInfo();
                    info.SndWarn = false;
                    info.SndSize = -1;
                    info.RcvWarn = false;
                    info.RcvSize = -1;
                    _setBufSizeWarn.Add(transport, info);
                }
                else
                {
                    info = _setBufSizeWarn[transport];
                }
                return info;
            }
        }

        // Returns the IClassFactory associated with this Slice type ID, not null if not found.
        internal Func<AnyClass>? FindClassFactory(string typeId) =>
            _classFactoryCache.GetOrAdd(typeId, typeId =>
            {
                string className = TypeIdToClassName(typeId);
                Type? factoryClass = FindType($"ZeroC.Ice.ClassFactory.{className}");
                if (factoryClass != null)
                {
                    MethodInfo? method = factoryClass.GetMethod("Create", BindingFlags.Public | BindingFlags.Static);
                    Debug.Assert(method != null);
                    return (Func<AnyClass>)Delegate.CreateDelegate(typeof(Func<AnyClass>), method);
                }
                return null;
            });

        internal Func<AnyClass>? FindClassFactory(int compactId) =>
           _compactIdCache.GetOrAdd(compactId, compactId =>
           {
               Type? factoryClass = FindType($"ZeroC.Ice.ClassFactory.CompactId_{compactId}");
               if (factoryClass != null)
               {
                   MethodInfo? method = factoryClass.GetMethod("Create", BindingFlags.Public | BindingFlags.Static);
                   Debug.Assert(method != null);
                   return (Func<AnyClass>)Delegate.CreateDelegate(typeof(Func<AnyClass>), method);
               }
               return null;
           });

        internal Func<string?, RemoteExceptionOrigin?, RemoteException>? FindRemoteExceptionFactory(string typeId) =>
            _remoteExceptionFactoryCache.GetOrAdd(typeId, typeId =>
            {
                string className = TypeIdToClassName(typeId);
                Type? factoryClass = FindType($"ZeroC.Ice.RemoteExceptionFactory.{className}");
                if (factoryClass != null)
                {
                    MethodInfo? method = factoryClass.GetMethod(
                        "Create",
                        BindingFlags.Public | BindingFlags.Static,
                        null,
                        CallingConventions.Any,
                        new Type[] { typeof(string), typeof(RemoteExceptionOrigin) },
                        null);
                    Debug.Assert(method != null);
                    return (Func<string?, RemoteExceptionOrigin?, RemoteException>)Delegate.CreateDelegate(
                        typeof(Func<string?, RemoteExceptionOrigin?, RemoteException>), method);
                }
                return null;
            });

        internal LocatorInfo? GetLocatorInfo(ILocatorPrx? locator)
        {
            // Returns locator info for a given locator. Automatically creates the locator info if it doesn't exist
            // yet.
            if (locator == null)
            {
                return null;
            }

            if (locator.Locator != null)
            {
                // The locator can't be located.
                locator = locator.Clone(clearLocator: true);
            }

            return _locatorInfoMap.GetOrAdd(locator,
                                            locator => new LocatorInfo(locator, _backgroundLocatorCacheUpdates));
        }

        internal RouterInfo? GetRouterInfo(IRouterPrx? router)
        {
            // Returns router info for a given router. Automatically creates the router info if it doesn't exist yet.
            if (router != null)
            {
                // The router cannot be routed.
                return _routerInfoTable.GetOrAdd(router.Clone(clearRouter: true), key => new RouterInfo(key));
            }
            else
            {
                return null;
            }
        }

        internal bool IncRetryBufferSize(int size)
        {
            lock (_mutex)
            {
                if (size + _retryBufferSize < RetryBufferSizeMax)
                {
                    _retryBufferSize += size;
                    return true;
                }
            }
            return false;
        }

        internal OutgoingConnectionFactory OutgoingConnectionFactory { get; }

        internal void SetRcvBufSizeWarn(Transport transport, int size)
        {
            lock (_setBufSizeWarn)
            {
                BufSizeWarnInfo info = GetBufSizeWarn(transport);
                info.RcvWarn = true;
                info.RcvSize = size;
                _setBufSizeWarn[transport] = info;
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
                    // Note that as soon as the process proxy is registered, the communicator might be shutdown by a
                    // remote client and admin facets might start receiving calls.
                    locator.GetRegistry()!.SetServerProcessProxy(serverId, process);
                }
                catch (Exception ex)
                {
                    if (TraceLevels.Locator >= 1)
                    {
                        Logger.Trace(TraceLevels.LocatorCategory,
                            $"could not register server `{serverId}' with the locator registry:\n{ex}");
                    }
                    throw;
                }

                if (TraceLevels.Locator >= 1)
                {
                    Logger.Trace(TraceLevels.LocatorCategory, $"registered server `{serverId}' with the locator registry");
                }
            }
        }

        internal void SetSndBufSizeWarn(Transport transport, int size)
        {
            lock (_setBufSizeWarn)
            {
                BufSizeWarnInfo info = GetBufSizeWarn(transport);
                info.SndWarn = true;
                info.SndSize = size;
                _setBufSizeWarn[transport] = info;
            }
        }

        internal void UpdateConnectionObservers()
        {
            try
            {
                OutgoingConnectionFactory.UpdateConnectionObservers();

                ObjectAdapter[] adapters;
                lock (_mutex)
                {
                    adapters = _adapters.ToArray();
                }

                foreach (ObjectAdapter adapter in adapters)
                {
                    adapter.UpdateConnectionObservers();
                }
            }
            catch (CommunicatorDisposedException)
            {
            }
        }

        private void AddAllAdminFacets()
        {
            lock (_mutex)
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

        private static Type? FindType(string csharpId)
        {
            Type? t;
            LoadAssemblies(); // Lazy initialization
            foreach (Assembly a in _loadedAssemblies.Values)
            {
                if ((t = a.GetType(csharpId)) != null)
                {
                    return t;
                }
            }
            return null;
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

        // Make sure that all assemblies that are referenced by this process are actually loaded. This is necessary so
        // we can use reflection on any type in any assembly because the type we are after will most likely not be in
        // the current assembly and, worse, may be in an assembly that has not been loaded yet. (Type.GetType() is no
        // good because it looks only in the calling object's assembly and mscorlib.dll.)
        private static void LoadAssemblies()
        {
            lock (_staticMutex)
            {
                Assembly[] assemblies = AppDomain.CurrentDomain.GetAssemblies();
                var newAssemblies = new List<Assembly>();
                foreach (Assembly assembly in assemblies)
                {
                    if (!_loadedAssemblies.ContainsKey(assembly.FullName!))
                    {
                        newAssemblies.Add(assembly);
                        _loadedAssemblies[assembly.FullName!] = assembly;
                    }
                }

                foreach (Assembly a in newAssemblies)
                {
                    LoadReferencedAssemblies(a);
                }
            }
        }

        private static void LoadReferencedAssemblies(Assembly a)
        {
            try
            {
                AssemblyName[] names = a.GetReferencedAssemblies();
                foreach (AssemblyName name in names)
                {
                    if (!_loadedAssemblies.ContainsKey(name.FullName))
                    {
                        try
                        {
                            var loadedAssembly = Assembly.Load(name);
                            // The value of name.FullName may not match that of loadedAssembly.FullName, so we record
                            // the assembly using both keys.
                            _loadedAssemblies[name.FullName] = loadedAssembly;
                            _loadedAssemblies[loadedAssembly.FullName!] = loadedAssembly;
                            LoadReferencedAssemblies(loadedAssembly);
                        }
                        catch (Exception)
                        {
                            // Ignore assemblies that cannot be loaded.
                        }
                    }
                }
            }
            catch (PlatformNotSupportedException)
            {
                // Some platforms like UWP do not support using GetReferencedAssemblies
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
    }
}
