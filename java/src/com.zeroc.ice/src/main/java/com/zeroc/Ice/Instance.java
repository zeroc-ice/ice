// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import com.zeroc.Ice.Instrumentation.CommunicatorObserver;
import com.zeroc.Ice.Instrumentation.ObserverUpdater;
import com.zeroc.Ice.Instrumentation.ThreadObserver;
import com.zeroc.Ice.Instrumentation.ThreadState;
import com.zeroc.Ice.SSL.EndpointFactoryI;
import com.zeroc.Ice.SSL.SSLEngine;

import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.PrintStream;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.UUID;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ScheduledThreadPoolExecutor;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;

/**
 * @hidden Public because it's used by SSL.
 */
public final class Instance {
    private static class ThreadObserverHelper {
        ThreadObserverHelper(String threadName) {
            _threadName = threadName;
        }

        public synchronized void updateObserver(
                CommunicatorObserver obsv) {
            assert (obsv != null);

            _observer =
                obsv.getThreadObserver(
                    "Communicator", _threadName, ThreadState.ThreadStateIdle, _observer);
            if (_observer != null) {
                _observer.attach();
            }
        }

        protected void beforeExecute() {
            _threadObserver = _observer;
            if (_threadObserver != null) {
                _threadObserver.stateChanged(
                    ThreadState.ThreadStateIdle, ThreadState.ThreadStateInUseForOther);
            }
        }

        protected void afterExecute() {
            if (_threadObserver != null) {
                _threadObserver.stateChanged(
                    ThreadState.ThreadStateInUseForOther, ThreadState.ThreadStateIdle);
                _threadObserver = null;
            }
        }

        private final String _threadName;
        //
        // We use a volatile to avoid synchronization when reading
        // _observer. Reference assignment is atomic in Java so it
        // also doesn't need to be synchronized.
        //
        private volatile ThreadObserver _observer;
        private ThreadObserver _threadObserver;
    }

    private static class Timer extends ScheduledThreadPoolExecutor {
        Timer(Properties props, String threadName) {
            super(1, Util.createThreadFactory(props, threadName)); // Single thread executor
            setRemoveOnCancelPolicy(true);
            setExecuteExistingDelayedTasksAfterShutdownPolicy(false);
            _observerHelper = new ThreadObserverHelper(threadName);
        }

        public void updateObserver(CommunicatorObserver obsv) {
            _observerHelper.updateObserver(obsv);
        }

        @Override
        protected void beforeExecute(Thread t, Runnable r) {
            _observerHelper.beforeExecute();
        }

        @Override
        protected void afterExecute(Runnable t, Throwable e) {
            _observerHelper.afterExecute();
        }

        private final ThreadObserverHelper _observerHelper;
    }

    private static class QueueExecutor extends ThreadPoolExecutor {
        QueueExecutor(Properties props, String threadName) {
            super(
                1,
                1,
                0,
                TimeUnit.MILLISECONDS,
                new LinkedBlockingQueue<Runnable>(),
                Util.createThreadFactory(props, threadName));
            _observerHelper = new ThreadObserverHelper(threadName);
        }

        public void updateObserver(CommunicatorObserver obsv) {
            _observerHelper.updateObserver(obsv);
        }

        @Override
        protected void beforeExecute(Thread t, Runnable r) {
            _observerHelper.beforeExecute();
        }

        @Override
        protected void afterExecute(Runnable t, Throwable e) {
            _observerHelper.afterExecute();
        }

        public void destroy() throws InterruptedException {
            shutdown();
            while (!isTerminated()) {
                // A very long time.
                awaitTermination(100000, TimeUnit.SECONDS);
            }
        }

        private final ThreadObserverHelper _observerHelper;
    }

    private class ObserverUpdaterI implements ObserverUpdater {
        @Override
        public void updateConnectionObservers() {
            Instance.this.updateConnectionObservers();
        }

        @Override
        public void updateThreadObservers() {
            Instance.this.updateThreadObservers();
        }
    }

    public InitializationData initializationData() {
        //
        // No check for destruction. It must be possible to access the initialization data after
        // destruction.
        //
        // No mutex lock, immutable.
        //
        return _initData;
    }

    public TraceLevels traceLevels() {
        // No mutex lock, immutable.
        assert (_traceLevels != null);
        return _traceLevels;
    }

    public DefaultsAndOverrides defaultsAndOverrides() {
        // No mutex lock, immutable.
        assert (_defaultsAndOverrides != null);
        return _defaultsAndOverrides;
    }

    public synchronized RouterManager routerManager() {
        if (_state == StateDestroyed) {
            throw new CommunicatorDestroyedException();
        }

        assert (_routerManager != null);
        return _routerManager;
    }

    public synchronized LocatorManager locatorManager() {
        if (_state == StateDestroyed) {
            throw new CommunicatorDestroyedException();
        }

        assert (_locatorManager != null);
        return _locatorManager;
    }

    public synchronized ReferenceFactory referenceFactory() {
        if (_state == StateDestroyed) {
            throw new CommunicatorDestroyedException();
        }

        assert (_referenceFactory != null);
        return _referenceFactory;
    }

    public synchronized OutgoingConnectionFactory outgoingConnectionFactory() {
        if (_state == StateDestroyed) {
            throw new CommunicatorDestroyedException();
        }

        assert (_outgoingConnectionFactory != null);
        return _outgoingConnectionFactory;
    }

    public synchronized ObjectAdapterFactory objectAdapterFactory() {
        if (_state == StateDestroyed) {
            throw new CommunicatorDestroyedException();
        }

        assert (_objectAdapterFactory != null);
        return _objectAdapterFactory;
    }

    public int protocolSupport() {
        return _protocolSupport;
    }

    public boolean preferIPv6() {
        return _preferIPv6;
    }

    public NetworkProxy networkProxy() {
        return _networkProxy;
    }

    public synchronized ThreadPool clientThreadPool() {
        if (_state == StateDestroyed) {
            throw new CommunicatorDestroyedException();
        }

        assert (_clientThreadPool != null);
        return _clientThreadPool;
    }

    public synchronized ThreadPool serverThreadPool() {
        if (_state == StateDestroyed) {
            throw new CommunicatorDestroyedException();
        }

        if (_serverThreadPool == null) // Lazy initialization.
            {
                if (_state == StateDestroyInProgress) {
                    throw new CommunicatorDestroyedException();
                }

                int timeout = _initData.properties.getIcePropertyAsInt("Ice.ServerIdleTime");
                _serverThreadPool = new ThreadPool(this, "Ice.ThreadPool.Server", timeout);
            }

        return _serverThreadPool;
    }

    public synchronized EndpointHostResolver endpointHostResolver() {
        if (_state == StateDestroyed) {
            throw new CommunicatorDestroyedException();
        }

        assert (_endpointHostResolver != null);
        return _endpointHostResolver;
    }

    public synchronized RetryQueue retryQueue() {
        if (_state == StateDestroyed) {
            throw new CommunicatorDestroyedException();
        }

        assert (_retryQueue != null);
        return _retryQueue;
    }

    public int[] retryIntervals() {
        // No mutex lock, we return a copy.
        return _retryIntervals.clone();
    }

    public synchronized ScheduledExecutorService timer() {
        if (_state == StateDestroyed) {
            throw new CommunicatorDestroyedException();
        }

        assert (_timer != null);
        return _timer;
    }

    synchronized EndpointFactoryManager endpointFactoryManager() {
        if (_state == StateDestroyed) {
            throw new CommunicatorDestroyedException();
        }

        assert (_endpointFactoryManager != null);
        return _endpointFactoryManager;
    }

    public synchronized PluginManager pluginManager() {
        if (_state == StateDestroyed) {
            throw new CommunicatorDestroyedException();
        }

        assert (_pluginManager != null);
        return _pluginManager;
    }

    public int messageSizeMax() {
        // No mutex lock, immutable.
        return _messageSizeMax;
    }

    public int batchAutoFlushSize() {
        // No mutex lock, immutable.
        return _batchAutoFlushSize;
    }

    public int classGraphDepthMax() {
        // No mutex lock, immutable.
        return _classGraphDepthMax;
    }

    public ToStringMode toStringMode() {
        // No mutex lock, immutable
        return _toStringMode;
    }

    public int cacheMessageBuffers() {
        // No mutex lock, immutable.
        return _cacheMessageBuffers;
    }

    public ImplicitContextI getImplicitContext() {
        return _implicitContext;
    }

    public synchronized ObjectPrx createAdmin(ObjectAdapter adminAdapter, Identity adminIdentity) {
        boolean createAdapter = adminAdapter == null;

        synchronized (this) {
            if (_state == StateDestroyed) {
                throw new CommunicatorDestroyedException();
            }

            if (adminIdentity == null
                || adminIdentity.name == null
                || adminIdentity.name.isEmpty()) {
                throw new IllegalArgumentException(
                    "The admin identity '" + adminIdentity + "' is not valid");
            }

            if (_adminAdapter != null) {
                throw new InitializationException("Admin already created");
            }

            if (!_adminEnabled) {
                throw new InitializationException("Admin is disabled");
            }

            if (createAdapter) {
                if (!_initData.properties.getIceProperty("Ice.Admin.Endpoints").isEmpty()) {
                    adminAdapter =
                        _objectAdapterFactory.createObjectAdapter("Ice.Admin", null, null);
                } else {
                    throw new InitializationException("Ice.Admin.Endpoints is not set");
                }
            }

            _adminIdentity = adminIdentity;
            _adminAdapter = adminAdapter;
            addAllAdminFacets();
        }

        if (createAdapter) {
            try {
                adminAdapter.activate();
            } catch (LocalException ex) {
                //
                // We cleanup _adminAdapter, however this error is not recoverable
                // (can't call again getAdmin() after fixing the problem)
                // since all the facets (servants) in the adapter are lost
                //
                adminAdapter.destroy();
                synchronized (this) {
                    _adminAdapter = null;
                }
                throw ex;
            }
        }
        setServerProcessProxy(adminAdapter, adminIdentity);
        return adminAdapter.createProxy(adminIdentity);
    }

    public ObjectPrx getAdmin() {
        ObjectAdapter adminAdapter;
        Identity adminIdentity;

        synchronized (this) {
            if (_state == StateDestroyed) {
                throw new CommunicatorDestroyedException();
            }

            if (_adminAdapter != null) {
                return _adminAdapter.createProxy(_adminIdentity);
            } else if (_adminEnabled) {
                if (!_initData.properties.getIceProperty("Ice.Admin.Endpoints").isEmpty()) {
                    adminAdapter =
                        _objectAdapterFactory.createObjectAdapter("Ice.Admin", null, null);
                } else {
                    return null;
                }
                adminIdentity =
                    new Identity(
                        "admin",
                        _initData.properties.getIceProperty("Ice.Admin.InstanceName"));
                if (adminIdentity.category.isEmpty()) {
                    adminIdentity.category = UUID.randomUUID().toString();
                }

                _adminIdentity = adminIdentity;
                _adminAdapter = adminAdapter;
                addAllAdminFacets();
                // continue below outside synchronization
            } else {
                return null;
            }
        }

        try {
            adminAdapter.activate();
        } catch (LocalException ex) {
            //
            // We cleanup _adminAdapter, however this error is not recoverable
            // (can't call again getAdmin() after fixing the problem)
            // since all the facets (servants) in the adapter are lost
            //
            adminAdapter.destroy();
            synchronized (this) {
                _adminAdapter = null;
            }
            throw ex;
        }

        setServerProcessProxy(adminAdapter, adminIdentity);
        return adminAdapter.createProxy(adminIdentity);
    }

    public synchronized void addAdminFacet(Object servant, String facet) {
        if (_state == StateDestroyed) {
            throw new CommunicatorDestroyedException();
        }

        if (_adminAdapter == null
            || (!_adminFacetFilter.isEmpty() && !_adminFacetFilter.contains(facet))) {
            if (_adminFacets.get(facet) != null) {
                throw new AlreadyRegisteredException("facet", facet);
            }
            _adminFacets.put(facet, servant);
        } else {
            _adminAdapter.addFacet(servant, _adminIdentity, facet);
        }
    }

    public synchronized Object removeAdminFacet(String facet) {
        if (_state == StateDestroyed) {
            throw new CommunicatorDestroyedException();
        }

        Object result;

        if (_adminAdapter == null
            || (!_adminFacetFilter.isEmpty() && !_adminFacetFilter.contains(facet))) {
            result = _adminFacets.remove(facet);
            if (result == null) {
                throw new NotRegisteredException("facet", facet);
            }
        } else {
            result = _adminAdapter.removeFacet(_adminIdentity, facet);
        }

        return result;
    }

    public synchronized Object findAdminFacet(String facet) {
        if (_state == StateDestroyed) {
            throw new CommunicatorDestroyedException();
        }

        Object result = null;

        if (_adminAdapter == null
            || (!_adminFacetFilter.isEmpty() && !_adminFacetFilter.contains(facet))) {
            result = _adminFacets.get(facet);
        } else {
            result = _adminAdapter.findFacet(_adminIdentity, facet);
        }

        return result;
    }

    public synchronized Map<String, Object> findAllAdminFacets() {
        if (_state == StateDestroyed) {
            throw new CommunicatorDestroyedException();
        }

        if (_adminAdapter == null) {
            return new HashMap<>(_adminFacets);
        } else {
            Map<String, Object> result = _adminAdapter.findAllFacets(_adminIdentity);
            if (!_adminFacets.isEmpty()) {
                // Also returns filtered facets
                result.putAll(_adminFacets);
            }
            return result;
        }
    }

    public synchronized void setDefaultLocator(LocatorPrx locator) {
        if (_state == StateDestroyed) {
            throw new CommunicatorDestroyedException();
        }

        _referenceFactory = _referenceFactory.setDefaultLocator(locator);
    }

    public synchronized void setDefaultRouter(RouterPrx router) {
        if (_state == StateDestroyed) {
            throw new CommunicatorDestroyedException();
        }

        _referenceFactory = _referenceFactory.setDefaultRouter(router);
    }

    public void setLogger(Logger logger) {
        //
        // No locking, as it can only be called during plug-in loading
        //
        _initData.logger = logger;
    }

    public void setThreadHooks(Runnable threadStart, Runnable threadStop) {
        //
        // No locking, as it can only be called during plug-in loading
        //
        _initData.threadStart = threadStart;
        _initData.threadStop = threadStop;
    }

    Class<?> findClass(String className) {
        return Util.findClass(className, _initData.classLoader);
    }

    // Only for use by com.zeroc.Ice.Communicator. If the application provides an initData, the initData argument is a
    // clone.
    void initialize(Communicator communicator, InitializationData initData) {
        _state = StateActive;
        _initData = initData;

        try {
            if (_initData.properties == null) {
                _initData.properties = new Properties();
            }

            Properties properties = _initData.properties;

            synchronized (Instance.class) {
                if (!_oneOffDone) {
                    String stdOut = properties.getIceProperty("Ice.StdOut");
                    String stdErr = properties.getIceProperty("Ice.StdErr");

                    PrintStream outStream = null;

                    if (!stdOut.isEmpty()) {
                        //
                        // We need to close the existing stdout for JVM thread dump to go to the new
                        // file
                        //
                        System.out.close();

                        try {
                            outStream =
                                new PrintStream(
                                    new FileOutputStream(stdOut, true));
                        } catch (FileNotFoundException ex) {
                            throw new FileException("cannot append to '" + stdOut + "'", ex);
                        }

                        System.setOut(outStream);
                    }
                    if (!stdErr.isEmpty()) {
                        //
                        // close for consistency with stdout
                        //
                        System.err.close();

                        if (stdErr.equals(stdOut)) {
                            System.setErr(outStream);
                        } else {
                            try {
                                System.setErr(
                                    new PrintStream(
                                        new FileOutputStream(stdErr, true)));
                            } catch (FileNotFoundException ex) {
                                throw new FileException("cannot append to '" + stdErr + "'", ex);
                            }
                        }
                    }
                    _oneOffDone = true;
                }
            }

            if (_initData.logger == null) {
                String logFile = properties.getIceProperty("Ice.LogFile");
                if (properties.getIcePropertyAsInt("Ice.UseSyslog") > 0
                    && !System.getProperty("os.name").startsWith("Windows")) {
                    if (!logFile.isEmpty()) {
                        throw new InitializationException(
                            "Both syslog and file logger cannot be enabled.");
                    }
                    _initData.logger =
                        new SysLoggerI(
                            properties.getIceProperty("Ice.ProgramName"),
                            properties.getIceProperty("Ice.SyslogFacility"),
                            properties.getIceProperty("Ice.SyslogHost"),
                            properties.getIcePropertyAsInt("Ice.SyslogPort"));
                } else if (!logFile.isEmpty()) {
                    _initData.logger =
                        new FileLoggerI(properties.getIceProperty("Ice.ProgramName"), logFile);
                } else {
                    _initData.logger = Util.getProcessLogger();
                    if (_initData.logger instanceof LoggerI) {
                        _initData.logger =
                            new LoggerI(properties.getIceProperty("Ice.ProgramName"));
                    }
                }
            }

            validatePackages();

            _traceLevels = new TraceLevels(properties);

            _defaultsAndOverrides = new DefaultsAndOverrides(properties);

            _clientConnectionOptions = readConnectionOptions("Ice.Connection.Client");
            _serverConnectionOptions = readConnectionOptions("Ice.Connection.Server");

            {
                int num = properties.getIcePropertyAsInt("Ice.MessageSizeMax");
                if (num < 1 || num > 0x7fffffff / 1024) {
                    _messageSizeMax = 0x7fffffff;
                } else {
                    _messageSizeMax =
                        num * 1024; // Property is in kilobytes, _messageSizeMax in bytes
                }
            }

            if (properties.getIceProperty("Ice.BatchAutoFlushSize").isEmpty()
                && !properties.getIceProperty("Ice.BatchAutoFlush").isEmpty()) {
                if (properties.getIcePropertyAsInt("Ice.BatchAutoFlush") > 0) {
                    _batchAutoFlushSize = _messageSizeMax;
                } else {
                    _batchAutoFlushSize = 0;
                }
            } else {
                int num = properties.getIcePropertyAsInt("Ice.BatchAutoFlushSize"); // 1MB
                if (num < 1) {
                    _batchAutoFlushSize = num;
                } else if (num > 0x7fffffff / 1024) {
                    _batchAutoFlushSize = 0x7fffffff;
                } else {
                    _batchAutoFlushSize =
                        num * 1024; // Property is in kilobytes, _batchAutoFlushSize in bytes
                }
            }

            {
                var num = properties.getIcePropertyAsInt("Ice.ClassGraphDepthMax");
                if (num < 1 || num > 0x7fffffff) {
                    _classGraphDepthMax = 0x7fffffff;
                } else {
                    _classGraphDepthMax = num;
                }
            }

            // Update _initData.sliceLoader.

            var sliceLoader = new CompositeSliceLoader();

            if (_initData.sliceLoader != null) {
                sliceLoader.add(_initData.sliceLoader);
            }

            final int notFoundCacheSize =
                properties.getIcePropertyAsInt("Ice.SliceLoader.NotFoundCacheSize");

            if (notFoundCacheSize <= 0) {
               _initData.sliceLoader = sliceLoader;
            } else {
                final Logger cacheFullLogger =
                    properties.getIcePropertyAsInt("Ice.Warn.SliceLoader") > 0 ? _initData.logger : null;

                _initData.sliceLoader = new NotFoundSliceLoaderDecorator(
                    sliceLoader,
                    notFoundCacheSize,
                    cacheFullLogger);
            }

            // Ice.Package.module loader.
            String packagePrefix = "Ice.Package";
            Map<String, String> moduleDict = properties.getPropertiesForPrefix(packagePrefix);
            if (!moduleDict.isEmpty()) {
                Map<String, String> moduleToPackageMap = new HashMap<>();
                for (Map.Entry<String, String> entry : moduleDict.entrySet()) {
                    String moduleName = entry.getKey().substring(packagePrefix.length() + 1); // top-level module
                    moduleToPackageMap.put("::" + moduleName, entry.getValue() + "." + moduleName);
                }
                sliceLoader.add(new ModuleToPackageSliceLoader(moduleToPackageMap, _initData.classLoader));
            }

            // Default package loader.
            String defaultPackage = properties.getIceProperty("Ice.Default.Package");
            if (!defaultPackage.isEmpty()) {
                sliceLoader.add(new DefaultPackageSliceLoader(defaultPackage, _initData.classLoader));
            }

            // Built-in modules loader.
            sliceLoader.add(
                new ModuleToPackageSliceLoader(
                    Map.of(
                        "::Glacier2", "com.zeroc.Glacier2",
                        "::Ice", "com.zeroc.Ice",
                        "::IceMX", "com.zeroc.Ice.IceMX",
                        "::IceBox", "com.zeroc.IceBox",
                        "::IceGrid", "com.zeroc.IceGrid",
                        "::IceStorm", "com.zeroc.IceStorm"),
                    _initData.classLoader));

            // Empty package prefix: module ::VisitorCenter maps to package VisitorCenter.
            sliceLoader.add(new DefaultPackageSliceLoader("", _initData.classLoader));

            String toStringModeStr = properties.getIceProperty("Ice.ToStringMode");
            if ("Unicode".equals(toStringModeStr)) {
                _toStringMode = ToStringMode.Unicode;
            } else if ("ASCII".equals(toStringModeStr)) {
                _toStringMode = ToStringMode.ASCII;
            } else if ("Compat".equals(toStringModeStr)) {
                _toStringMode = ToStringMode.Compat;
            } else {
                throw new InitializationException(
                    "The value for Ice.ToStringMode must be Unicode, ASCII or Compat");
            }

            _implicitContext =
                ImplicitContextI.create(properties.getIceProperty("Ice.ImplicitContext"));

            _routerManager = new RouterManager();

            _locatorManager = new LocatorManager(properties);

            _referenceFactory = new ReferenceFactory(this, communicator);

            boolean isIPv6Supported = Network.isIPv6Supported();
            boolean ipv4 = properties.getIcePropertyAsInt("Ice.IPv4") > 0;
            boolean ipv6 = isIPv6Supported ? properties.getIcePropertyAsInt("Ice.IPv6") > 0 : false;

            if (!ipv4 && !ipv6) {
                throw new InitializationException("Both IPV4 and IPv6 support cannot be disabled.");
            } else if (ipv4 && ipv6) {
                _protocolSupport = Network.EnableBoth;
            } else if (ipv4) {
                _protocolSupport = Network.EnableIPv4;
            } else {
                _protocolSupport = Network.EnableIPv6;
            }
            _preferIPv6 = properties.getIcePropertyAsInt("Ice.PreferIPv6Address") > 0;

            _networkProxy = createNetworkProxy(properties, _protocolSupport);

            _sslEngine = new SSLEngine(communicator);
            _endpointFactoryManager = new EndpointFactoryManager(this);

            ProtocolInstance tcpProtocol =
                new ProtocolInstance(this, TCPEndpointType.value, "tcp", false);
            _endpointFactoryManager.add(new TcpEndpointFactory(tcpProtocol));

            ProtocolInstance udpProtocol =
                new ProtocolInstance(this, UDPEndpointType.value, "udp", false);
            _endpointFactoryManager.add(new UdpEndpointFactory(udpProtocol));

            com.zeroc.Ice.SSL.Instance sslInstance =
                new com.zeroc.Ice.SSL.Instance(_sslEngine, SSLEndpointType.value, "ssl");
            _endpointFactoryManager.add(
                new EndpointFactoryI(sslInstance, TCPEndpointType.value));

            ProtocolInstance wsProtocol =
                new ProtocolInstance(this, WSEndpointType.value, "ws", false);
            _endpointFactoryManager.add(new WSEndpointFactory(wsProtocol, TCPEndpointType.value));

            ProtocolInstance wssProtocol =
                new ProtocolInstance(this, WSSEndpointType.value, "wss", true);
            _endpointFactoryManager.add(new WSEndpointFactory(wssProtocol, SSLEndpointType.value));

            _pluginManager = new PluginManagerI(communicator, this);

            if (_initData.valueFactoryManager == null) {
                _initData.valueFactoryManager = new ValueFactoryManagerI();
            }

            _outgoingConnectionFactory = new OutgoingConnectionFactory(communicator, this);

            _objectAdapterFactory = new ObjectAdapterFactory(this, communicator);

            _retryQueue = new RetryQueue(this);

            String[] arr = properties.getIcePropertyAsList("Ice.RetryIntervals");
            if (arr.length == 0) {
                _retryIntervals = new int[]{0};
            } else {
                _retryIntervals = new int[arr.length];

                for (int i = 0; i < arr.length; i++) {
                    int v;

                    try {
                        v = Integer.parseInt(arr[i]);
                    } catch (NumberFormatException ex) {
                        v = 0;
                    }

                    // If -1 is the first value, no retry and wait intervals.
                    if (i == 0 && v == -1) {
                        _retryIntervals = new int[0];
                        break;
                    }

                    _retryIntervals[i] = v > 0 ? v : 0;
                }
            }
            _cacheMessageBuffers = properties.getIcePropertyAsInt("Ice.CacheMessageBuffers");
        } catch (Exception ex) {
            destroy(false);
            throw ex;
        }
    }

    @SuppressWarnings({"nofinalizer", "deprecation"})
    @Override
    protected synchronized void finalize() throws Throwable {
        try {
            Assert.FinalizerAssert(_state == StateDestroyed);
            Assert.FinalizerAssert(_referenceFactory == null);
            Assert.FinalizerAssert(_outgoingConnectionFactory == null);
            Assert.FinalizerAssert(_objectAdapterFactory == null);
            Assert.FinalizerAssert(_clientThreadPool == null);
            Assert.FinalizerAssert(_serverThreadPool == null);
            Assert.FinalizerAssert(_endpointHostResolver == null);
            Assert.FinalizerAssert(_timer == null);
            Assert.FinalizerAssert(_routerManager == null);
            Assert.FinalizerAssert(_locatorManager == null);
            Assert.FinalizerAssert(_endpointFactoryManager == null);
            Assert.FinalizerAssert(_pluginManager == null);
            Assert.FinalizerAssert(_retryQueue == null);
        } catch (Exception ex) {} finally {
            super.finalize();
        }
    }

    public String[] finishSetup(String[] args, Communicator communicator) {

        Properties properties = _initData.properties;
        //
        // Load plug-ins.
        //
        assert (_serverThreadPool == null);
        PluginManagerI pluginManagerImpl = (PluginManagerI) _pluginManager;
        args = pluginManagerImpl.loadPlugins(args);

        //
        // Initialize the endpoint factories once all the plugins are loaded. This gives
        // the opportunity for the endpoint factories to find underlying factories.
        //
        _endpointFactoryManager.initialize();

        //
        // Create Admin facets, if enabled.
        //
        // Note that any logger-dependent admin facet must be created after we load all plugins,
        // since one of these plugins can be a Logger plugin that sets a new logger during loading
        //

        if (properties.getIceProperty("Ice.Admin.Enabled").isEmpty()) {
            _adminEnabled = !properties.getIceProperty("Ice.Admin.Endpoints").isEmpty();
        } else {
            _adminEnabled = properties.getIcePropertyAsInt("Ice.Admin.Enabled") > 0;
        }

        String[] facetFilter = properties.getIcePropertyAsList("Ice.Admin.Facets");
        if (facetFilter.length > 0) {
            _adminFacetFilter.addAll(Arrays.asList(facetFilter));
        }

        if (_adminEnabled) {
            //
            // Process facet
            //
            String processFacetName = "Process";
            if (_adminFacetFilter.isEmpty() || _adminFacetFilter.contains(processFacetName)) {
                _adminFacets.put(processFacetName, new ProcessI(communicator));
            }

            //
            // Logger facet
            //
            String loggerFacetName = "Logger";
            if (_adminFacetFilter.isEmpty() || _adminFacetFilter.contains(loggerFacetName)) {
                LoggerAdminLogger logger = new LoggerAdminLoggerI(properties, _initData.logger);
                setLogger(logger);
                _adminFacets.put(loggerFacetName, logger.getFacet());
            }

            //
            // Properties facet
            //
            String propertiesFacetName = "Properties";
            PropertiesAdminI propsAdmin = null;
            if (_adminFacetFilter.isEmpty() || _adminFacetFilter.contains(propertiesFacetName)) {
                propsAdmin = new PropertiesAdminI(this);
                _adminFacets.put(propertiesFacetName, propsAdmin);
            }

            //
            // Metrics facet
            //
            String metricsFacetName = "Metrics";
            if (_adminFacetFilter.isEmpty() || _adminFacetFilter.contains(metricsFacetName)) {
                CommunicatorObserverI observer = new CommunicatorObserverI(_initData);
                _initData.observer = observer;
                _adminFacets.put(metricsFacetName, observer.getFacet());

                //
                // Make sure the admin plugin receives property updates.
                //
                if (propsAdmin != null) {
                    propsAdmin.addUpdateCallback(observer.getFacet());
                }
            }
        }

        //
        // Set observer updater
        //
        if (_initData.observer != null) {
            _initData.observer.setObserverUpdater(new ObserverUpdaterI());
        }

        //
        // Create threads.
        //
        try {
            _timer = new Timer(properties, Util.createThreadName(properties, "Ice.Timer"));
        } catch (RuntimeException ex) {
            String s = "cannot create thread for timer:\n" + Ex.toString(ex);
            _initData.logger.error(s);
            throw ex;
        }

        try {
            _endpointHostResolver = new EndpointHostResolver(this);
        } catch (RuntimeException ex) {
            String s = "cannot create thread for endpoint host resolver:\n" + Ex.toString(ex);
            _initData.logger.error(s);
            throw ex;
        }

        _clientThreadPool = new ThreadPool(this, "Ice.ThreadPool.Client", 0);

        //
        // The default router/locator may have been set during the loading of plugins. Therefore we
        // make sure it is not already set before checking the property.
        //
        if (_referenceFactory.getDefaultRouter() == null) {
            RouterPrx router =
                RouterPrx.uncheckedCast(communicator.propertyToProxy("Ice.Default.Router"));
            if (router != null) {
                _referenceFactory = _referenceFactory.setDefaultRouter(router);
            }
        }

        if (_referenceFactory.getDefaultLocator() == null) {
            LocatorPrx loc =
                LocatorPrx.uncheckedCast(communicator.propertyToProxy("Ice.Default.Locator"));
            if (loc != null) {
                _referenceFactory = _referenceFactory.setDefaultLocator(loc);
            }
        }

        // SslEngine initialization
        _sslEngine.initialize();

        //
        // Server thread pool initialization is lazy in serverThreadPool().
        //

        //
        // An application can set Ice.InitPlugins=0 if it wants to postpone
        // initialization until after it has interacted directly with the
        // plug-ins.
        //
        if (properties.getIcePropertyAsInt("Ice.InitPlugins") > 0) {
            pluginManagerImpl.initializePlugins();
        }

        //
        // This must be done last as this call creates the Ice.Admin object adapter
        // and eventually registers a process proxy with the Ice locator (allowing
        // remote clients to invoke on Ice.Admin facets as soon as it's registered).
        //
        if (properties.getIcePropertyAsInt("Ice.Admin.DelayCreation") <= 0) {
            getAdmin();
        }

        return args;
    }

    //
    // Only for use by com.zeroc.Ice.Communicator
    //
    void destroy(boolean interruptible) {
        synchronized (this) {
            //
            // If destroy is in progress, wait for it to be done. This
            // is necessary in case destroy() is called concurrently
            // by multiple threads.
            //
            while (_state == StateDestroyInProgress) {
                try {
                    wait();
                } catch (InterruptedException ex) {
                    if (interruptible) {
                        throw new OperationInterruptedException(ex);
                    }
                }
            }

            if (_state == StateDestroyed) {
                return;
            }
            _state = StateDestroyInProgress;
        }

        try {
            //
            // Shutdown and destroy all the incoming and outgoing Ice connections and wait for the
            // connections to be finished.
            //
            if (_objectAdapterFactory != null) {
                _objectAdapterFactory.shutdown();
            }

            if (_outgoingConnectionFactory != null) {
                _outgoingConnectionFactory.destroy();
            }

            if (_objectAdapterFactory != null) {
                _objectAdapterFactory.destroy();
            }

            if (_outgoingConnectionFactory != null) {
                _outgoingConnectionFactory.waitUntilFinished();
            }

            if (_retryQueue != null) {
                _retryQueue.destroy(); // Must be called before destroying thread pools.
            }

            if (_initData.observer != null) {
                _initData.observer.setObserverUpdater(null);
            }

            if (_initData.logger instanceof LoggerAdminLogger) {
                // This only disables the remote logging; we don't set or reset _initData.logger
                ((LoggerAdminLogger) _initData.logger).destroy();
            }

            //
            // Now, destroy the thread pools. This must be done *only* after all the connections are
            // finished (the connections destruction can require invoking callbacks with the thread
            // pools).
            //
            if (_serverThreadPool != null) {
                _serverThreadPool.destroy();
            }
            if (_clientThreadPool != null) {
                _clientThreadPool.destroy();
            }
            if (_endpointHostResolver != null) {
                _endpointHostResolver.destroy();
            }
            if (_timer != null) {
                _timer.shutdown(); // Don't use shutdownNow(), timers don't support interrupts
            }

            //
            // Wait for all the threads to be finished.
            //
            try {
                if (_clientThreadPool != null) {
                    _clientThreadPool.joinWithAllThreads();
                }
                if (_serverThreadPool != null) {
                    _serverThreadPool.joinWithAllThreads();
                }
                if (_endpointHostResolver != null) {
                    _endpointHostResolver.joinWithThread();
                }
                if (_timer != null) {
                    while (!_timer.isTerminated()) {
                        // A very long time.
                        _timer.awaitTermination(100000, TimeUnit.SECONDS);
                    }
                }
            } catch (InterruptedException ex) {
                if (interruptible) {
                    throw new OperationInterruptedException(ex);
                }
            }

            //
            // NOTE: at this point destroy() can't be interrupted
            // anymore. The calls below are therefore guaranteed to be called once.
            //
            if (_routerManager != null) {
                _routerManager.destroy();
            }

            if (_locatorManager != null) {
                _locatorManager.destroy();
            }

            if (_initData.properties.getIcePropertyAsInt("Ice.Warn.UnusedProperties") > 0) {
                List<String> unusedProperties =
                    _initData.properties.getUnusedProperties();
                if (!unusedProperties.isEmpty()) {
                    StringBuilder message =
                        new StringBuilder("The following properties were set but never read:");
                    for (String p : unusedProperties) {
                        message.append("\n    ");
                        message.append(p);
                    }
                    _initData.logger.warning(message.toString());
                }
            }

            //
            // Destroy last so that a Logger plugin can receive all log/traces before its
            // destruction.
            //
            if (_pluginManager != null) {
                _pluginManager.destroy();
            }

            if (_initData.logger instanceof FileLoggerI) {
                FileLoggerI logger = (FileLoggerI) _initData.logger;
                logger.destroy();
            }

            if (_initData.logger instanceof SysLoggerI) {
                SysLoggerI logger = (SysLoggerI) _initData.logger;
                logger.destroy();
            }

            synchronized (this) {
                _objectAdapterFactory = null;
                _outgoingConnectionFactory = null;
                _retryQueue = null;

                _serverThreadPool = null;
                _clientThreadPool = null;
                _endpointHostResolver = null;
                _timer = null;

                _referenceFactory = null;
                _routerManager = null;
                _locatorManager = null;
                _endpointFactoryManager = null;

                _pluginManager = null;

                _adminAdapter = null;
                _adminFacets.clear();

                _state = StateDestroyed;
                notifyAll();
            }
        } finally {
            synchronized (this) {
                if (_state == StateDestroyInProgress) {
                    assert interruptible;
                    _state = StateActive;
                    notifyAll();
                }
            }
        }
    }

    public BufSizeWarnInfo getBufSizeWarn(short type) {
        synchronized (_setBufSizeWarn) {
            BufSizeWarnInfo info;
            if (!_setBufSizeWarn.containsKey(type)) {
                info = new BufSizeWarnInfo();
                info.sndWarn = false;
                info.sndSize = -1;
                info.rcvWarn = false;
                info.rcvSize = -1;
                _setBufSizeWarn.put(type, info);
            } else {
                info = _setBufSizeWarn.get(type);
            }
            return info;
        }
    }

    public void setSndBufSizeWarn(short type, int size) {
        synchronized (_setBufSizeWarn) {
            BufSizeWarnInfo info = getBufSizeWarn(type);
            info.sndWarn = true;
            info.sndSize = size;
            _setBufSizeWarn.put(type, info);
        }
    }

    public void setRcvBufSizeWarn(short type, int size) {
        synchronized (_setBufSizeWarn) {
            BufSizeWarnInfo info = getBufSizeWarn(type);
            info.rcvWarn = true;
            info.rcvSize = size;
            _setBufSizeWarn.put(type, info);
        }
    }

    SliceLoader sliceLoader() {
        return _initData.sliceLoader;
    }

    ConnectionOptions clientConnectionOptions() {
        return _clientConnectionOptions;
    }

    ConnectionOptions serverConnectionOptions(String adapterName) {
        assert (!adapterName.isEmpty());
        Properties properties = _initData.properties;
        String propertyPrefix = adapterName + ".Connection";

        return new ConnectionOptions(
            properties.getPropertyAsIntWithDefault(
                propertyPrefix + ".ConnectTimeout",
                _serverConnectionOptions.connectTimeout()),
            properties.getPropertyAsIntWithDefault(
                propertyPrefix + ".CloseTimeout", _serverConnectionOptions.closeTimeout()),
            properties.getPropertyAsIntWithDefault(
                propertyPrefix + ".IdleTimeout", _serverConnectionOptions.idleTimeout()),
            properties.getPropertyAsIntWithDefault(
                propertyPrefix + ".EnableIdleCheck",
                _serverConnectionOptions.enableIdleCheck() ? 1 : 0)
                > 0,
            properties.getPropertyAsIntWithDefault(
                propertyPrefix + ".InactivityTimeout",
                _serverConnectionOptions.inactivityTimeout()),
            properties.getPropertyAsIntWithDefault(
                propertyPrefix + ".MaxDispatches",
                _serverConnectionOptions.maxDispatches()));
    }

    private void updateConnectionObservers() {
        try {
            assert (_outgoingConnectionFactory != null);
            _outgoingConnectionFactory.updateConnectionObservers();
            assert (_objectAdapterFactory != null);
            _objectAdapterFactory.updateConnectionObservers();
        } catch (CommunicatorDestroyedException ex) {}
    }

    private void updateThreadObservers() {
        try {
            if (_clientThreadPool != null) {
                _clientThreadPool.updateObservers();
            }
            if (_serverThreadPool != null) {
                _serverThreadPool.updateObservers();
            }
            assert (_objectAdapterFactory != null);
            _objectAdapterFactory.updateThreadObservers();
            if (_endpointHostResolver != null) {
                _endpointHostResolver.updateObserver();
            }
            if (_timer != null) {
                _timer.updateObserver(_initData.observer);
            }
        } catch (CommunicatorDestroyedException ex) {}
    }

    // TODO: should we drop this method together with the _Marker classes?
    private void validatePackages() {
        final String prefix = "Ice.Package.";
        Map<String, String> map = _initData.properties.getPropertiesForPrefix(prefix);
        for (Map.Entry<String, String> p : map.entrySet()) {
            String key = p.getKey();
            String pkg = p.getValue();
            if (key.length() == prefix.length()) {
                _initData.logger.warning("ignoring invalid property: " + key + "=" + pkg);
            }
            String module = key.substring(prefix.length());
            String className = pkg + "." + module + "._Marker";
            Class<?> cls = null;
            try {
                cls = findClass(className);
            } catch (Exception ex) {}
            if (cls == null) {
                _initData.logger.warning("unable to validate package: " + key + "=" + pkg);
            }
        }
    }

    private synchronized void addAllAdminFacets() {
        Map<String, Object> filteredFacets = new HashMap<>();
        for (Map.Entry<String, Object> p : _adminFacets.entrySet()) {
            if (_adminFacetFilter.isEmpty() || _adminFacetFilter.contains(p.getKey())) {
                _adminAdapter.addFacet(p.getValue(), _adminIdentity, p.getKey());
            } else {
                filteredFacets.put(p.getKey(), p.getValue());
            }
        }
        _adminFacets = filteredFacets;
    }

    private void setServerProcessProxy(ObjectAdapter adminAdapter, Identity adminIdentity) {
        ObjectPrx admin = adminAdapter.createProxy(adminIdentity);
        LocatorPrx locator = adminAdapter.getLocator();
        String serverId = _initData.properties.getIceProperty("Ice.Admin.ServerId");

        if (locator != null && !serverId.isEmpty()) {
            ProcessPrx process = ProcessPrx.uncheckedCast(admin.ice_facet("Process"));
            try {
                //
                // Note that as soon as the process proxy is registered, the communicator might be
                // shutdown by a remote client and admin facets might start receiving calls.
                //
                locator.getRegistry().setServerProcessProxy(serverId, process);
            } catch (ServerNotFoundException ex) {
                if (_traceLevels.location >= 1) {
                    StringBuilder s = new StringBuilder(128);
                    s.append("couldn't register server `");
                    s.append(serverId);
                    s.append("' with the locator registry:\n");
                    s.append("the server is not known to the locator registry");
                    _initData.logger.trace(_traceLevels.locationCat, s.toString());
                }

                throw new InitializationException(
                    "Locator knows nothing about server `" + serverId + "'");
            } catch (LocalException ex) {
                if (_traceLevels.location >= 1) {
                    StringBuilder s = new StringBuilder(128);
                    s.append("couldn't register server `");
                    s.append(serverId);
                    s.append("' with the locator registry:\n");
                    s.append(ex.toString());
                    _initData.logger.trace(_traceLevels.locationCat, s.toString());
                }
                throw ex;
            }

            if (_traceLevels.location >= 1) {
                StringBuilder s = new StringBuilder(128);
                s.append("registered server `");
                s.append(serverId);
                s.append("' with the locator registry");
                _initData.logger.trace(_traceLevels.locationCat, s.toString());
            }
        }
    }

    private NetworkProxy createNetworkProxy(Properties properties, int protocolSupport) {
        String proxyHost;

        proxyHost = properties.getIceProperty("Ice.SOCKSProxyHost");
        if (!proxyHost.isEmpty()) {
            if (protocolSupport == Network.EnableIPv6) {
                throw new InitializationException("IPv6 only is not supported with SOCKS4 proxies");
            }
            int proxyPort = properties.getIcePropertyAsInt("Ice.SOCKSProxyPort");
            return new SOCKSNetworkProxy(proxyHost, proxyPort);
        }

        proxyHost = properties.getIceProperty("Ice.HTTPProxyHost");
        if (!proxyHost.isEmpty()) {
            return new HTTPNetworkProxy(
                proxyHost, properties.getIcePropertyAsInt("Ice.HTTPProxyPort"));
        }

        return null;
    }

    private ConnectionOptions readConnectionOptions(String propertyPrefix) {
        Properties properties = _initData.properties;
        return new ConnectionOptions(
            properties.getIcePropertyAsInt(propertyPrefix + ".ConnectTimeout"),
            properties.getIcePropertyAsInt(propertyPrefix + ".CloseTimeout"),
            properties.getIcePropertyAsInt(propertyPrefix + ".IdleTimeout"),
            properties.getIcePropertyAsInt(propertyPrefix + ".EnableIdleCheck") > 0,
            properties.getIcePropertyAsInt(propertyPrefix + ".InactivityTimeout"),
            properties.getIcePropertyAsInt(propertyPrefix + ".MaxDispatches"));
    }

    private static final int StateActive = 0;
    private static final int StateDestroyInProgress = 1;
    private static final int StateDestroyed = 2;
    private int _state;

    private InitializationData _initData; // Immutable, not reset by destroy().
    private TraceLevels _traceLevels; // Immutable, not reset by destroy().
    private DefaultsAndOverrides _defaultsAndOverrides; // Immutable, not reset by destroy().
    private int _messageSizeMax; // Immutable, not reset by destroy().
    private int _batchAutoFlushSize; // Immutable, not reset by destroy().
    private int _classGraphDepthMax; // Immutable, not reset by destroy().
    private SliceLoader _sliceLoader; // Immutable, not reset by destroy().
    private ToStringMode _toStringMode; // Immutable, not reset by destroy().
    private int _cacheMessageBuffers; // Immutable, not reset by destroy().
    private ImplicitContextI _implicitContext;
    private RouterManager _routerManager;
    private LocatorManager _locatorManager;
    private ReferenceFactory _referenceFactory;
    private OutgoingConnectionFactory _outgoingConnectionFactory;
    private ObjectAdapterFactory _objectAdapterFactory;
    private int _protocolSupport;
    private boolean _preferIPv6;
    private NetworkProxy _networkProxy;
    private ThreadPool _clientThreadPool;
    private ThreadPool _serverThreadPool;
    private EndpointHostResolver _endpointHostResolver;
    private RetryQueue _retryQueue;
    private int[] _retryIntervals;
    private Timer _timer;
    private EndpointFactoryManager _endpointFactoryManager;
    private PluginManager _pluginManager;

    private boolean _adminEnabled;
    private ObjectAdapter _adminAdapter;
    private Map<String, Object> _adminFacets = new HashMap<>();
    private final Set<String> _adminFacetFilter = new HashSet<>();
    private Identity _adminIdentity;
    private final Map<Short, BufSizeWarnInfo> _setBufSizeWarn = new HashMap<>();

    private static boolean _oneOffDone;
    private SSLEngine _sslEngine;

    private ConnectionOptions _clientConnectionOptions;
    private ConnectionOptions _serverConnectionOptions;
}
