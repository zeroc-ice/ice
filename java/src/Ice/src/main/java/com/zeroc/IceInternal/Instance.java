// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceInternal;

import java.util.concurrent.TimeUnit;

import com.zeroc.Ice.Instrumentation.ThreadState;

public final class Instance implements com.zeroc.Ice.ClassResolver
{
    static private class ThreadObserverHelper
    {
        ThreadObserverHelper(String threadName)
        {
            _threadName = threadName;
        }

        synchronized public void updateObserver(com.zeroc.Ice.Instrumentation.CommunicatorObserver obsv)
        {
            assert(obsv != null);

            _observer = obsv.getThreadObserver("Communicator", _threadName, ThreadState.ThreadStateIdle, _observer);
            if(_observer != null)
            {
                _observer.attach();
            }
        }

        protected void beforeExecute()
        {
            _threadObserver = _observer;
            if(_threadObserver != null)
            {
                _threadObserver.stateChanged(ThreadState.ThreadStateIdle, ThreadState.ThreadStateInUseForOther);
            }
        }

        protected void afterExecute()
        {
            if(_threadObserver != null)
            {
                _threadObserver.stateChanged(ThreadState.ThreadStateInUseForOther, ThreadState.ThreadStateIdle);
                _threadObserver = null;
            }
        }

        final private String _threadName;
        //
        // We use a volatile to avoid synchronization when reading
        // _observer. Reference assignement is atomic in Java so it
        // also doesn't need to be synchronized.
        //
        private volatile com.zeroc.Ice.Instrumentation.ThreadObserver _observer;
        private com.zeroc.Ice.Instrumentation.ThreadObserver _threadObserver;
    }

    static private class Timer extends java.util.concurrent.ScheduledThreadPoolExecutor
    {
        Timer(com.zeroc.Ice.Properties props, String threadName)
        {
            super(1, Util.createThreadFactory(props, threadName)); // Single thread executor
            if(!Util.isAndroid())
            {
                // This API doesn't exist on Android up to API level 20.
                setRemoveOnCancelPolicy(true);
            }
            setExecuteExistingDelayedTasksAfterShutdownPolicy(false);
            _observerHelper = new ThreadObserverHelper(threadName);
        }

        public void updateObserver(com.zeroc.Ice.Instrumentation.CommunicatorObserver obsv)
        {
            _observerHelper.updateObserver(obsv);
        }

        @Override
        protected void beforeExecute(Thread t, Runnable r)
        {
            _observerHelper.beforeExecute();
        }

        @Override
        protected void afterExecute(Runnable t, Throwable e)
        {
            _observerHelper.afterExecute();
        }

        private final ThreadObserverHelper _observerHelper;
    }

    static private class QueueExecutor extends java.util.concurrent.ThreadPoolExecutor
    {
        QueueExecutor(com.zeroc.Ice.Properties props, String threadName)
        {
            super(1, 1, 0, TimeUnit.MILLISECONDS, new java.util.concurrent.LinkedBlockingQueue<Runnable>(),
                  Util.createThreadFactory(props, threadName));
            _observerHelper = new ThreadObserverHelper(threadName);
        }

        public void updateObserver(com.zeroc.Ice.Instrumentation.CommunicatorObserver obsv)
        {
            _observerHelper.updateObserver(obsv);
        }

        @Override
        protected void beforeExecute(Thread t, Runnable r)
        {
            _observerHelper.beforeExecute();
        }

        @Override
        protected void afterExecute(Runnable t, Throwable e)
        {
            _observerHelper.afterExecute();
        }

        public void destroy()
            throws InterruptedException
        {
            shutdown();
            while(!isTerminated())
            {
                // A very long time.
                awaitTermination(100000, java.util.concurrent.TimeUnit.SECONDS);
            }
        }

        private final ThreadObserverHelper _observerHelper;
    }

    private class ObserverUpdaterI implements com.zeroc.Ice.Instrumentation.ObserverUpdater
    {
        @Override
        public void
        updateConnectionObservers()
        {
            Instance.this.updateConnectionObservers();
        }

        @Override
        public void
        updateThreadObservers()
        {
            Instance.this.updateThreadObservers();
        }
    }

    public com.zeroc.Ice.InitializationData
    initializationData()
    {
        //
        // No check for destruction. It must be possible to access the
        // initialization data after destruction.
        //
        // No mutex lock, immutable.
        //
        return _initData;
    }

    public TraceLevels
    traceLevels()
    {
        // No mutex lock, immutable.
        assert(_traceLevels != null);
        return _traceLevels;
    }

    public DefaultsAndOverrides
    defaultsAndOverrides()
    {
        // No mutex lock, immutable.
        assert(_defaultsAndOverrides != null);
        return _defaultsAndOverrides;
    }

    public synchronized RouterManager
    routerManager()
    {
        if(_state == StateDestroyed)
        {
            throw new com.zeroc.Ice.CommunicatorDestroyedException();
        }

        assert(_routerManager != null);
        return _routerManager;
    }

    public synchronized LocatorManager
    locatorManager()
    {
        if(_state == StateDestroyed)
        {
            throw new com.zeroc.Ice.CommunicatorDestroyedException();
        }

        assert(_locatorManager != null);
        return _locatorManager;
    }

    public synchronized ReferenceFactory
    referenceFactory()
    {
        if(_state == StateDestroyed)
        {
            throw new com.zeroc.Ice.CommunicatorDestroyedException();
        }

        assert(_referenceFactory != null);
        return _referenceFactory;
    }

    public synchronized RequestHandlerFactory
    requestHandlerFactory()
    {
        if(_state == StateDestroyed)
        {
            throw new com.zeroc.Ice.CommunicatorDestroyedException();
        }

        assert(_requestHandlerFactory != null);
        return _requestHandlerFactory;
    }

    public synchronized ProxyFactory
    proxyFactory()
    {
        if(_state == StateDestroyed)
        {
            throw new com.zeroc.Ice.CommunicatorDestroyedException();
        }

        assert(_proxyFactory != null);
        return _proxyFactory;
    }

    public synchronized OutgoingConnectionFactory
    outgoingConnectionFactory()
    {
        if(_state == StateDestroyed)
        {
            throw new com.zeroc.Ice.CommunicatorDestroyedException();
        }

        assert(_outgoingConnectionFactory != null);
        return _outgoingConnectionFactory;
    }

    public synchronized ObjectAdapterFactory
    objectAdapterFactory()
    {
        if(_state == StateDestroyed)
        {
            throw new com.zeroc.Ice.CommunicatorDestroyedException();
        }

        assert(_objectAdapterFactory != null);
        return _objectAdapterFactory;
    }

    public int
    protocolSupport()
    {
        return _protocolSupport;
    }

    public boolean
    preferIPv6()
    {
        return _preferIPv6;
    }

    public NetworkProxy
    networkProxy()
    {
        return _networkProxy;
    }

    public synchronized ThreadPool
    clientThreadPool()
    {
        if(_state == StateDestroyed)
        {
            throw new com.zeroc.Ice.CommunicatorDestroyedException();
        }

        assert(_clientThreadPool != null);
        return _clientThreadPool;
    }

    public synchronized ThreadPool
    serverThreadPool()
    {
        if(_state == StateDestroyed)
        {
            throw new com.zeroc.Ice.CommunicatorDestroyedException();
        }

        if(_serverThreadPool == null) // Lazy initialization.
        {
            if(_state == StateDestroyInProgress)
            {
                throw new com.zeroc.Ice.CommunicatorDestroyedException();
            }

            int timeout = _initData.properties.getPropertyAsInt("Ice.ServerIdleTime");
            _serverThreadPool = new ThreadPool(this, "Ice.ThreadPool.Server", timeout);
        }

        return _serverThreadPool;
    }

    public synchronized EndpointHostResolver
    endpointHostResolver()
    {
        if(_state == StateDestroyed)
        {
            throw new com.zeroc.Ice.CommunicatorDestroyedException();
        }

        assert(_endpointHostResolver != null);
        return _endpointHostResolver;
    }

    synchronized public RetryQueue
    retryQueue()
    {
        if(_state == StateDestroyed)
        {
            throw new com.zeroc.Ice.CommunicatorDestroyedException();
        }

        assert(_retryQueue != null);
        return _retryQueue;
    }

    synchronized public java.util.concurrent.ScheduledExecutorService
    timer()
    {
        if(_state == StateDestroyed)
        {
            throw new com.zeroc.Ice.CommunicatorDestroyedException();
        }

        assert(_timer != null);
        return _timer;
    }

    public synchronized EndpointFactoryManager
    endpointFactoryManager()
    {
        if(_state == StateDestroyed)
        {
            throw new com.zeroc.Ice.CommunicatorDestroyedException();
        }

        assert(_endpointFactoryManager != null);
        return _endpointFactoryManager;
    }

    public synchronized com.zeroc.Ice.PluginManager
    pluginManager()
    {
        if(_state == StateDestroyed)
        {
            throw new com.zeroc.Ice.CommunicatorDestroyedException();
        }

        assert(_pluginManager != null);
        return _pluginManager;
    }

    public int
    messageSizeMax()
    {
        // No mutex lock, immutable.
        return _messageSizeMax;
    }

    public int
    batchAutoFlushSize()
    {
        // No mutex lock, immutable.
        return _batchAutoFlushSize;
    }

    public com.zeroc.Ice.ToStringMode
    toStringMode()
    {
        // No mutex lock, immutable
        return _toStringMode;
    }

    public int
    cacheMessageBuffers()
    {
        // No mutex lock, immutable.
        return _cacheMessageBuffers;
    }

    public ACMConfig
    clientACM()
    {
        // No mutex lock, immutable.
        return _clientACM;
    }

    public ACMConfig
    serverACM()
    {
        // No mutex lock, immutable.
        return _serverACM;
    }

    public com.zeroc.Ice.ImplicitContextI
    getImplicitContext()
    {
        return _implicitContext;
    }

    public synchronized com.zeroc.Ice.ObjectPrx
    createAdmin(com.zeroc.Ice.ObjectAdapter adminAdapter, com.zeroc.Ice.Identity adminIdentity)
    {
        if(Thread.interrupted())
        {
            throw new com.zeroc.Ice.OperationInterruptedException();
        }

        boolean createAdapter = (adminAdapter == null);

        synchronized(this)
        {
            if(_state == StateDestroyed)
            {
                throw new com.zeroc.Ice.CommunicatorDestroyedException();
            }

            if(adminIdentity == null || adminIdentity.name == null || adminIdentity.name.isEmpty())
            {
                throw new com.zeroc.Ice.IllegalIdentityException(adminIdentity);
            }

            if(_adminAdapter != null)
            {
                throw new com.zeroc.Ice.InitializationException("Admin already created");
            }

            if(!_adminEnabled)
            {
                throw new com.zeroc.Ice.InitializationException("Admin is disabled");
            }

            if(createAdapter)
            {
                if(!_initData.properties.getProperty("Ice.Admin.Endpoints").isEmpty())
                {
                    adminAdapter = _objectAdapterFactory.createObjectAdapter("Ice.Admin", null);
                }
                else
                {
                    throw new com.zeroc.Ice.InitializationException("Ice.Admin.Endpoints is not set");
                }
            }

            _adminIdentity = adminIdentity;
            _adminAdapter = adminAdapter;
            addAllAdminFacets();
        }

        if(createAdapter)
        {
            try
            {
                adminAdapter.activate();
            }
            catch(com.zeroc.Ice.LocalException ex)
            {
                //
                // We cleanup _adminAdapter, however this error is not recoverable
                // (can't call again getAdmin() after fixing the problem)
                // since all the facets (servants) in the adapter are lost
                //
                adminAdapter.destroy();
                synchronized(this)
                {
                    _adminAdapter = null;
                }
                throw ex;
            }
        }
        setServerProcessProxy(adminAdapter, adminIdentity);
        return adminAdapter.createProxy(adminIdentity);
    }

    public com.zeroc.Ice.ObjectPrx
    getAdmin()
    {
        if(Thread.interrupted())
        {
            throw new com.zeroc.Ice.OperationInterruptedException();
        }

        com.zeroc.Ice.ObjectAdapter adminAdapter;
        com.zeroc.Ice.Identity adminIdentity;

        synchronized(this)
        {
            if(_state == StateDestroyed)
            {
                throw new com.zeroc.Ice.CommunicatorDestroyedException();
            }

            if(_adminAdapter != null)
            {
                return _adminAdapter.createProxy(_adminIdentity);
            }
            else if(_adminEnabled)
            {
                if(!_initData.properties.getProperty("Ice.Admin.Endpoints").isEmpty())
                {
                    adminAdapter = _objectAdapterFactory.createObjectAdapter("Ice.Admin", null);
                }
                else
                {
                    return null;
                }
                adminIdentity =
                    new com.zeroc.Ice.Identity("admin", _initData.properties.getProperty("Ice.Admin.InstanceName"));
                if(adminIdentity.category.isEmpty())
                {
                    adminIdentity.category = java.util.UUID.randomUUID().toString();
                }

                _adminIdentity = adminIdentity;
                _adminAdapter = adminAdapter;
                addAllAdminFacets();
                // continue below outside synchronization
            }
            else
            {
                return null;
            }
        }

        try
        {
            adminAdapter.activate();
        }
        catch(com.zeroc.Ice.LocalException ex)
        {
            //
            // We cleanup _adminAdapter, however this error is not recoverable
            // (can't call again getAdmin() after fixing the problem)
            // since all the facets (servants) in the adapter are lost
            //
            adminAdapter.destroy();
            synchronized(this)
            {
                _adminAdapter = null;
            }
            throw ex;
        }

        setServerProcessProxy(adminAdapter, adminIdentity);
        return adminAdapter.createProxy(adminIdentity);
    }

    public synchronized void
    addAdminFacet(com.zeroc.Ice.Object servant, String facet)
    {
        if(_state == StateDestroyed)
        {
            throw new com.zeroc.Ice.CommunicatorDestroyedException();
        }

        if(_adminAdapter == null || (!_adminFacetFilter.isEmpty() && !_adminFacetFilter.contains(facet)))
        {
            if(_adminFacets.get(facet) != null)
            {
                throw new com.zeroc.Ice.AlreadyRegisteredException("facet", facet);
            }
            _adminFacets.put(facet, servant);
        }
        else
        {
            _adminAdapter.addFacet(servant, _adminIdentity, facet);
        }
    }

    public synchronized com.zeroc.Ice.Object
    removeAdminFacet(String facet)
    {
        if(_state == StateDestroyed)
        {
            throw new com.zeroc.Ice.CommunicatorDestroyedException();
        }

        com.zeroc.Ice.Object result;

        if(_adminAdapter == null || (!_adminFacetFilter.isEmpty() && !_adminFacetFilter.contains(facet)))
        {
            result = _adminFacets.remove(facet);
            if(result == null)
            {
                throw new com.zeroc.Ice.NotRegisteredException("facet", facet);
            }
        }
        else
        {
            result = _adminAdapter.removeFacet(_adminIdentity, facet);
        }

        return result;
    }

    public synchronized com.zeroc.Ice.Object
    findAdminFacet(String facet)
    {
        if(_state == StateDestroyed)
        {
            throw new com.zeroc.Ice.CommunicatorDestroyedException();
        }

        com.zeroc.Ice.Object result = null;

        if(_adminAdapter == null || (!_adminFacetFilter.isEmpty() && !_adminFacetFilter.contains(facet)))
        {
            result = _adminFacets.get(facet);
        }
        else
        {
            result = _adminAdapter.findFacet(_adminIdentity, facet);
        }

        return result;
    }

    public synchronized java.util.Map<String, com.zeroc.Ice.Object>
    findAllAdminFacets()
    {
        if(_state == StateDestroyed)
        {
            throw new com.zeroc.Ice.CommunicatorDestroyedException();
        }

        if(_adminAdapter == null)
        {
            return new java.util.HashMap<>(_adminFacets);
        }
        else
        {
            java.util.Map<String, com.zeroc.Ice.Object> result = _adminAdapter.findAllFacets(_adminIdentity);
            if(!_adminFacets.isEmpty())
            {
                // Also returns filtered facets
                result.putAll(_adminFacets);
            }
            return result;
        }
    }

    public synchronized void
    setDefaultLocator(com.zeroc.Ice.LocatorPrx locator)
    {
        if(_state == StateDestroyed)
        {
            throw new com.zeroc.Ice.CommunicatorDestroyedException();
        }

        _referenceFactory = _referenceFactory.setDefaultLocator(locator);
    }

    public synchronized void
    setDefaultRouter(com.zeroc.Ice.RouterPrx router)
    {
        if(_state == StateDestroyed)
        {
            throw new com.zeroc.Ice.CommunicatorDestroyedException();
        }

        _referenceFactory = _referenceFactory.setDefaultRouter(router);
    }

    public void
    setLogger(com.zeroc.Ice.Logger logger)
    {
        //
        // No locking, as it can only be called during plug-in loading
        //
        _initData.logger = logger;
    }

    public void
    setThreadHook(com.zeroc.Ice.ThreadNotification threadHook)
    {
        //
        // No locking, as it can only be called during plug-in loading
        //
        _initData.threadHook = threadHook;
    }

    public Class<?>
    findClass(String className)
    {
        return Util.findClass(className, _initData.classLoader);
    }

    public ClassLoader
    getClassLoader()
    {
        return _initData.classLoader;
    }

    static private String[] _iceTypeIdPrefixes =
    {
        "::Glacier2::",
        "::Ice::",
        "::IceBox::",
        "::IceDiscovery::",
        "::IceGrid::",
        "::IceLocatorDiscovery::",
        "::IceMX::",
        "::IcePatch2::",
        "::IceStorm::"
    };

    //
    // From com.zeroc.Ice.ClassResolver.
    //
    public Class<?> resolveClass(String typeId)
        throws LinkageError
    {
        Class<?> c = null;

        //
        // To convert a Slice type id into a Java class, we do the following:
        //
        // 1. Convert the Slice type id into a classname (e.g., ::M::X -> M.X).
        // 2. If that fails, extract the top-level module (if any) from the type id
        //    and check for a Package property. If found, prepend the property
        //    value to the classname.
        // 3. If that fails, check for an Default.Package property. If found,
        //    prepend the property value to the classname.
        //
        String className;
        boolean addClass = false;

        //
        // See if we've already translated this type ID before.
        //
        synchronized(this)
        {
            className = _typeToClassMap.get(typeId);
        }

        //
        // It's a new type ID, so first convert it into a Java class name.
        //
        if(className == null)
        {
            className = com.zeroc.Ice.Util.typeIdToClass(typeId);
            addClass = true;
        }

        //
        // See if we can find the class without any prefix.
        //
        c = getConcreteClass(className);

        //
        // See if the application defined an Ice.Package.MODULE property.
        //
        if(c == null)
        {
            int pos = typeId.indexOf(':', 2);
            if(pos != -1)
            {
                String topLevelModule = typeId.substring(2, pos);
                String pkg = _initData.properties.getProperty("Ice.Package." + topLevelModule);
                if(pkg.length() > 0)
                {
                    c = getConcreteClass(pkg + "." + className);
                }
            }
        }

        //
        // See if the application defined a default package.
        //
        if(c == null)
        {
            String pkg = _initData.properties.getProperty("Ice.Default.Package");
            if(pkg.length() > 0)
            {
                c = getConcreteClass(pkg + "." + className);
            }
        }

        //
        // See if the type ID is one of the Ice modules.
        //
        if(c == null)
        {
            String pkg = null;
            for(int i = 0; i < _iceTypeIdPrefixes.length && c == null; ++i)
            {
                if(typeId.startsWith(_iceTypeIdPrefixes[i]))
                {
                    c = getConcreteClass("com.zeroc." + className);
                }
            }
        }

        //
        // If we found the class, update our map so we don't have to translate this type ID again.
        //
        if(c != null && addClass)
        {
            synchronized(this)
            {
                className = c.getName();
                if(_typeToClassMap.containsKey(typeId))
                {
                    assert(_typeToClassMap.get(typeId).equals(className));
                }
                else
                {
                    _typeToClassMap.put(typeId, className);
                }
            }
        }

        return c;
    }

    public String resolveCompactId(int compactId)
    {
        String className = "com.zeroc.IceCompactId.TypeId_" + Integer.toString(compactId);
        Class<?> c = getConcreteClass(className);
        if(c == null)
        {
            for(String pkg : _packages)
            {
                c = getConcreteClass(pkg + "." + className);
                if(c != null)
                {
                    break;
                }
            }
        }
        if(c != null)
        {
            try
            {
                return (String)c.getField("typeId").get(null);
            }
            catch(Exception ex)
            {
                assert(false);
            }
        }
        return "";
    }

    public Class<?> getConcreteClass(String className)
        throws LinkageError
    {
        Class<?> c = findClass(className);

        if(c != null)
        {
            //
            // Ensure the class is instantiable. The constants are
            // defined in the JVM specification (0x200 = interface,
            // 0x400 = abstract).
            //
            final int modifiers = c.getModifiers();
            if((modifiers & 0x200) == 0 && (modifiers & 0x400) == 0)
            {
                return c;
            }
        }

        return null;
    }

    public boolean
    useApplicationClassLoader()
    {
        return _useApplicationClassLoader;
    }

    public boolean
    queueRequests()
    {
        return _queueExecutorService != null;
    }

    synchronized public QueueExecutorService
    getQueueExecutor()
    {
        if(_state == StateDestroyed)
        {
            throw new com.zeroc.Ice.CommunicatorDestroyedException();
        }
        return _queueExecutorService;
    }

    //
    // Only for use by com.zeroc.Ice.CommunicatorI
    //
    public
    Instance(com.zeroc.Ice.Communicator communicator, com.zeroc.Ice.InitializationData initData)
    {
        _state = StateActive;
        _initData = initData;

        try
        {
            if(_initData.properties == null)
            {
                _initData.properties = com.zeroc.Ice.Util.createProperties();
            }

            synchronized(Instance.class)
            {
                if(!_oneOffDone)
                {
                    String stdOut = _initData.properties.getProperty("Ice.StdOut");
                    String stdErr = _initData.properties.getProperty("Ice.StdErr");

                    java.io.PrintStream outStream = null;

                    if(stdOut.length() > 0)
                    {
                        //
                        // We need to close the existing stdout for JVM thread dump to go
                        // to the new file
                        //
                        System.out.close();

                        try
                        {
                            outStream = new java.io.PrintStream(new java.io.FileOutputStream(stdOut, true));
                        }
                        catch(java.io.FileNotFoundException ex)
                        {
                            throw new com.zeroc.Ice.FileException(0, stdOut, ex);
                        }

                        System.setOut(outStream);
                    }
                    if(stdErr.length() > 0)
                    {
                        //
                        // close for consistency with stdout
                        //
                        System.err.close();

                        if(stdErr.equals(stdOut))
                        {
                            System.setErr(outStream);
                        }
                        else
                        {
                            try
                            {
                                System.setErr(new java.io.PrintStream(new java.io.FileOutputStream(stdErr, true)));
                            }
                            catch(java.io.FileNotFoundException ex)
                            {
                                throw new com.zeroc.Ice.FileException(0, stdErr, ex);
                            }

                        }
                    }
                    _oneOffDone = true;
                }
            }

            if(_initData.logger == null)
            {
                String logfile = _initData.properties.getProperty("Ice.LogFile");
                if(_initData.properties.getPropertyAsInt("Ice.UseSyslog") > 0 &&
                   !System.getProperty("os.name").startsWith("Windows"))
                {
                    if(logfile.length() != 0)
                    {
                        throw new com.zeroc.Ice.InitializationException(
                            "Both syslog and file logger cannot be enabled.");
                    }
                    _initData.logger = new com.zeroc.Ice.SysLoggerI(
                        _initData.properties.getProperty("Ice.ProgramName"),
                        _initData.properties.getPropertyWithDefault("Ice.SyslogFacility", "LOG_USER"));
                }
                else if(logfile.length() != 0)
                {
                    _initData.logger =
                        new com.zeroc.Ice.LoggerI(_initData.properties.getProperty("Ice.ProgramName"), logfile);
                }
                else
                {
                    _initData.logger = com.zeroc.Ice.Util.getProcessLogger();
                }
            }

            _packages = validatePackages();

            _useApplicationClassLoader = _initData.properties.getPropertyAsInt("Ice.UseApplicationClassLoader") > 0;

            _traceLevels = new TraceLevels(_initData.properties);

            _defaultsAndOverrides = new DefaultsAndOverrides(_initData.properties, _initData.logger);

            _clientACM = new ACMConfig(_initData.properties,
                                       _initData.logger,
                                       "Ice.ACM.Client",
                                       new ACMConfig(_initData.properties, _initData.logger, "Ice.ACM",
                                                     new ACMConfig(false)));

            _serverACM = new ACMConfig(_initData.properties,
                                       _initData.logger,
                                       "Ice.ACM.Server",
                                       new ACMConfig(_initData.properties, _initData.logger, "Ice.ACM",
                                                     new ACMConfig(true)));

            {
                final int defaultMessageSizeMax = 1024;
                int num = _initData.properties.getPropertyAsIntWithDefault("Ice.MessageSizeMax", defaultMessageSizeMax);
                if(num < 1 || num > 0x7fffffff / 1024)
                {
                    _messageSizeMax = 0x7fffffff;
                }
                else
                {
                    _messageSizeMax = num * 1024; // Property is in kilobytes, _messageSizeMax in bytes
                }
            }

            if(_initData.properties.getProperty("Ice.BatchAutoFlushSize").isEmpty() &&
               !_initData.properties.getProperty("Ice.BatchAutoFlush").isEmpty())
            {
                if(_initData.properties.getPropertyAsInt("Ice.BatchAutoFlush") > 0)
                {
                    _batchAutoFlushSize = _messageSizeMax;
                }
                else
                {
                    _batchAutoFlushSize = 0;
                }
            }
            else
            {
                int num = _initData.properties.getPropertyAsIntWithDefault("Ice.BatchAutoFlushSize", 1024); // 1MB
                if(num < 1)
                {
                    _batchAutoFlushSize = num;
                }
                else if(num > 0x7fffffff / 1024)
                {
                    _batchAutoFlushSize = 0x7fffffff;
                }
                else
                {
                    _batchAutoFlushSize = num * 1024; // Property is in kilobytes, _batchAutoFlushSize in bytes
                }
            }

            String toStringModeStr = _initData.properties.getPropertyWithDefault("Ice.ToStringMode", "Unicode");
            if(toStringModeStr.equals("Unicode"))
            {
                _toStringMode = com.zeroc.Ice.ToStringMode.Unicode;
            }
            else if(toStringModeStr.equals("ASCII"))
            {
                _toStringMode = com.zeroc.Ice.ToStringMode.ASCII;
            }
            else if(toStringModeStr.equals("Compat"))
            {
                _toStringMode = com.zeroc.Ice.ToStringMode.Compat;
            }
            else
            {
                throw new com.zeroc.Ice.InitializationException("The value for Ice.ToStringMode must be Unicode, ASCII or Compat");
            }

            _implicitContext =
                com.zeroc.Ice.ImplicitContextI.create(_initData.properties.getProperty("Ice.ImplicitContext"));

            _routerManager = new RouterManager();

            _locatorManager = new LocatorManager(_initData.properties);

            _referenceFactory = new ReferenceFactory(this, communicator);

            _requestHandlerFactory = new RequestHandlerFactory(this);

            _proxyFactory = new ProxyFactory(this);

            boolean isIPv6Supported = Network.isIPv6Supported();
            boolean ipv4 = _initData.properties.getPropertyAsIntWithDefault("Ice.IPv4", 1) > 0;
            boolean ipv6 = _initData.properties.getPropertyAsIntWithDefault("Ice.IPv6", isIPv6Supported ? 1 : 0) > 0;
            if(!ipv4 && !ipv6)
            {
                throw new com.zeroc.Ice.InitializationException("Both IPV4 and IPv6 support cannot be disabled.");
            }
            else if(ipv4 && ipv6)
            {
                _protocolSupport = Network.EnableBoth;
            }
            else if(ipv4)
            {
                _protocolSupport = Network.EnableIPv4;
            }
            else
            {
                _protocolSupport = Network.EnableIPv6;
            }
            _preferIPv6 = _initData.properties.getPropertyAsInt("Ice.PreferIPv6Address") > 0;

            _networkProxy = createNetworkProxy(_initData.properties, _protocolSupport);

            _endpointFactoryManager = new EndpointFactoryManager(this);

            ProtocolInstance tcpProtocolInstance =
                new ProtocolInstance(this, com.zeroc.Ice.TCPEndpointType.value, "tcp", false);
            _endpointFactoryManager.add(new TcpEndpointFactory(tcpProtocolInstance));

            ProtocolInstance udpProtocolInstance =
                new ProtocolInstance(this, com.zeroc.Ice.UDPEndpointType.value, "udp", false);
            _endpointFactoryManager.add(new UdpEndpointFactory(udpProtocolInstance));

            _pluginManager = new com.zeroc.Ice.PluginManagerI(communicator, this);

            if(_initData.valueFactoryManager == null)
            {
                _initData.valueFactoryManager = new ValueFactoryManagerI();
            }

            _outgoingConnectionFactory = new OutgoingConnectionFactory(communicator, this);

            _objectAdapterFactory = new ObjectAdapterFactory(this, communicator);

            _retryQueue = new RetryQueue(this);

            //
            // If Ice.ThreadInterruptSafe is set or we're running on Android all
            // IO is done on the background thread. For Android we use the queue
            // executor as Android doesn't allow any network invocations on the main
            // thread even if the call is non-blocking.
            //
            if(_initData.properties.getPropertyAsInt("Ice.ThreadInterruptSafe") > 0 || Util.isAndroid())
            {
                _queueExecutor = new QueueExecutor(_initData.properties,
                                                   Util.createThreadName(_initData.properties, "Ice.BackgroundIO"));
                _queueExecutorService = new QueueExecutorService(_queueExecutor);

                // Caching message buffers is not supported with background IO.
                _cacheMessageBuffers = 0;
            }
            else
            {
                _cacheMessageBuffers = _initData.properties.getPropertyAsIntWithDefault("Ice.CacheMessageBuffers", 2);
            }
        }
        catch(com.zeroc.Ice.LocalException ex)
        {
            destroy(false);
            throw ex;
        }
    }

    @Override
    protected synchronized void
    finalize()
        throws Throwable
    {
        try
        {
            com.zeroc.IceUtilInternal.Assert.FinalizerAssert(_state == StateDestroyed);
            com.zeroc.IceUtilInternal.Assert.FinalizerAssert(_referenceFactory == null);
            com.zeroc.IceUtilInternal.Assert.FinalizerAssert(_requestHandlerFactory == null);
            com.zeroc.IceUtilInternal.Assert.FinalizerAssert(_proxyFactory == null);
            com.zeroc.IceUtilInternal.Assert.FinalizerAssert(_outgoingConnectionFactory == null);
            com.zeroc.IceUtilInternal.Assert.FinalizerAssert(_objectAdapterFactory == null);
            com.zeroc.IceUtilInternal.Assert.FinalizerAssert(_clientThreadPool == null);
            com.zeroc.IceUtilInternal.Assert.FinalizerAssert(_serverThreadPool == null);
            com.zeroc.IceUtilInternal.Assert.FinalizerAssert(_endpointHostResolver == null);
            com.zeroc.IceUtilInternal.Assert.FinalizerAssert(_timer == null);
            com.zeroc.IceUtilInternal.Assert.FinalizerAssert(_routerManager == null);
            com.zeroc.IceUtilInternal.Assert.FinalizerAssert(_locatorManager == null);
            com.zeroc.IceUtilInternal.Assert.FinalizerAssert(_endpointFactoryManager == null);
            com.zeroc.IceUtilInternal.Assert.FinalizerAssert(_pluginManager == null);
            com.zeroc.IceUtilInternal.Assert.FinalizerAssert(_retryQueue == null);
        }
        catch(java.lang.Exception ex)
        {
        }
        finally
        {
            super.finalize();
        }
    }

    public String[] finishSetup(String[] args, com.zeroc.Ice.Communicator communicator)
    {
        //
        // Load plug-ins.
        //
        assert(_serverThreadPool == null);
        com.zeroc.Ice.PluginManagerI pluginManagerImpl = (com.zeroc.Ice.PluginManagerI)_pluginManager;
        args = pluginManagerImpl.loadPlugins(args);

        //
        // Add WS and WSS endpoint factories if TCP/SSL factories are installed.
        //
        final EndpointFactory tcpFactory = _endpointFactoryManager.get(com.zeroc.Ice.TCPEndpointType.value);
        if(tcpFactory != null)
        {
            final ProtocolInstance instance =
                new ProtocolInstance(this, com.zeroc.Ice.WSEndpointType.value, "ws", false);
            _endpointFactoryManager.add(new WSEndpointFactory(instance, tcpFactory.clone(instance, null)));
        }
        final EndpointFactory sslFactory = _endpointFactoryManager.get(com.zeroc.Ice.SSLEndpointType.value);
        if(sslFactory != null)
        {
            final ProtocolInstance instance =
                new ProtocolInstance(this, com.zeroc.Ice.WSSEndpointType.value, "wss", true);
            _endpointFactoryManager.add(new WSEndpointFactory(instance, sslFactory.clone(instance, null)));
        }

        //
        // Create Admin facets, if enabled.
        //
        // Note that any logger-dependent admin facet must be created after we load all plugins,
        // since one of these plugins can be a Logger plugin that sets a new logger during loading
        //

        if(_initData.properties.getProperty("Ice.Admin.Enabled").isEmpty())
        {
            _adminEnabled = !_initData.properties.getProperty("Ice.Admin.Endpoints").isEmpty();
        }
        else
        {
            _adminEnabled = _initData.properties.getPropertyAsInt("Ice.Admin.Enabled") > 0;
        }

        String[] facetFilter = _initData.properties.getPropertyAsList("Ice.Admin.Facets");
        if(facetFilter.length > 0)
        {
            _adminFacetFilter.addAll(java.util.Arrays.asList(facetFilter));
        }

        if(_adminEnabled)
        {
            //
            // Process facet
            //
            String processFacetName = "Process";
            if(_adminFacetFilter.isEmpty() || _adminFacetFilter.contains(processFacetName))
            {
                _adminFacets.put(processFacetName, new ProcessI(communicator));
            }

            //
            // Logger facet
            //
            String loggerFacetName = "Logger";
            if(_adminFacetFilter.isEmpty() || _adminFacetFilter.contains(loggerFacetName))
            {
                LoggerAdminLogger logger = new LoggerAdminLoggerI(_initData.properties, _initData.logger);
                setLogger(logger);
                _adminFacets.put(loggerFacetName, logger.getFacet());
            }

            //
            // Properties facet
            //
            String propertiesFacetName = "Properties";
            PropertiesAdminI propsAdmin = null;
            if(_adminFacetFilter.isEmpty() || _adminFacetFilter.contains(propertiesFacetName))
            {
                propsAdmin = new PropertiesAdminI(this);
                _adminFacets.put(propertiesFacetName, propsAdmin);
            }

            //
            // Metrics facet
            //
            String metricsFacetName = "Metrics";
            if(_adminFacetFilter.isEmpty() || _adminFacetFilter.contains(metricsFacetName))
            {
                 CommunicatorObserverI observer = new CommunicatorObserverI(_initData);
                 _initData.observer = observer;
                 _adminFacets.put(metricsFacetName, observer.getFacet());

                 //
                 // Make sure the admin plugin receives property updates.
                 //
                 if(propsAdmin != null)
                 {
                     propsAdmin.addUpdateCallback(observer.getFacet());
                 }
            }
        }

        //
        // Set observer updater
        //
        if(_initData.observer != null)
        {
            _initData.observer.setObserverUpdater(new ObserverUpdaterI());
        }

        //
        // Create threads.
        //
        try
        {
            _timer = new Timer(_initData.properties, Util.createThreadName(_initData.properties, "Ice.Timer"));
        }
        catch(RuntimeException ex)
        {
            String s = "cannot create thread for timer:\n" + Ex.toString(ex);
            _initData.logger.error(s);
            throw ex;
        }

        try
        {
            _endpointHostResolver = new EndpointHostResolver(this);
        }
        catch(RuntimeException ex)
        {
            String s = "cannot create thread for endpoint host resolver:\n" + Ex.toString(ex);
            _initData.logger.error(s);
            throw ex;
        }

        _clientThreadPool = new ThreadPool(this, "Ice.ThreadPool.Client", 0);

        //
        // The default router/locator may have been set during the loading of plugins.
        // Therefore we make sure it is not already set before checking the property.
        //
        if(_referenceFactory.getDefaultRouter() == null)
        {
            com.zeroc.Ice.RouterPrx router =
                com.zeroc.Ice.RouterPrx.uncheckedCast(_proxyFactory.propertyToProxy("Ice.Default.Router"));
            if(router != null)
            {
                _referenceFactory = _referenceFactory.setDefaultRouter(router);
            }
        }

        if(_referenceFactory.getDefaultLocator() == null)
        {
            com.zeroc.Ice.LocatorPrx loc =
                com.zeroc.Ice.LocatorPrx.uncheckedCast(_proxyFactory.propertyToProxy("Ice.Default.Locator"));
            if(loc != null)
            {
                _referenceFactory = _referenceFactory.setDefaultLocator(loc);
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
        if(_initData.properties.getPropertyAsIntWithDefault("Ice.InitPlugins", 1) > 0)
        {
            pluginManagerImpl.initializePlugins();
        }

        //
        // This must be done last as this call creates the Ice.Admin object adapter
        // and eventually registers a process proxy with the Ice locator (allowing
        // remote clients to invoke on Ice.Admin facets as soon as it's registered).
        //
        if(_initData.properties.getPropertyAsIntWithDefault("Ice.Admin.DelayCreation", 0) <= 0)
        {
            getAdmin();
        }

        return args;
    }

    //
    // Only for use by com.zeroc.Ice.CommunicatorI
    //
    @SuppressWarnings("deprecation")
    public void
    destroy(boolean interruptible)
    {
        if(interruptible && Thread.interrupted())
        {
            throw new com.zeroc.Ice.OperationInterruptedException();
        }

        synchronized(this)
        {
            //
            // If destroy is in progress, wait for it to be done. This
            // is necessary in case destroy() is called concurrently
            // by multiple threads.
            //
            while(_state == StateDestroyInProgress)
            {
                try
                {
                    wait();
                }
                catch(InterruptedException ex)
                {
                    if(interruptible)
                    {
                        throw new com.zeroc.Ice.OperationInterruptedException();
                    }
                }
            }

            if(_state == StateDestroyed)
            {
                return;
            }
            _state = StateDestroyInProgress;
        }

        try
        {
            //
            // Shutdown and destroy all the incoming and outgoing Ice
            // connections and wait for the connections to be finished.
            //
            if(_objectAdapterFactory != null)
            {
                _objectAdapterFactory.shutdown();
            }

            if(_outgoingConnectionFactory != null)
            {
                _outgoingConnectionFactory.destroy();
            }

            if(_objectAdapterFactory != null)
            {
                _objectAdapterFactory.destroy();
            }

            if(_outgoingConnectionFactory != null)
            {
                _outgoingConnectionFactory.waitUntilFinished();
            }

            if(_retryQueue != null)
            {
                _retryQueue.destroy(); // Must be called before destroying thread pools.
            }

            if(_initData.observer != null)
            {
                _initData.observer.setObserverUpdater(null);
            }

            if(_initData.logger instanceof LoggerAdminLogger)
            {
                //
                // This only disables the remote logging; we don't set or reset _initData.logger
                //
                ((LoggerAdminLogger)_initData.logger).destroy();
            }

            //
            // Now, destroy the thread pools. This must be done *only* after
            // all the connections are finished (the connections destruction
            // can require invoking callbacks with the thread pools).
            //
            if(_serverThreadPool != null)
            {
                _serverThreadPool.destroy();
            }
            if(_clientThreadPool != null)
            {
                _clientThreadPool.destroy();
            }
            if(_endpointHostResolver != null)
            {
                _endpointHostResolver.destroy();
            }
            if(_timer != null)
            {
                _timer.shutdown(); // Don't use shutdownNow(), timers don't support interrupts
            }

            //
            // Wait for all the threads to be finished.
            //
            try
            {
                if(_clientThreadPool != null)
                {
                    _clientThreadPool.joinWithAllThreads();
                }
                if(_serverThreadPool != null)
                {
                    _serverThreadPool.joinWithAllThreads();
                }
                if(_endpointHostResolver != null)
                {
                    _endpointHostResolver.joinWithThread();
                }
                if(_queueExecutor != null)
                {
                    _queueExecutor.destroy();
                }
                if(_timer != null)
                {
                    while(!_timer.isTerminated())
                    {
                        // A very long time.
                        _timer.awaitTermination(100000, java.util.concurrent.TimeUnit.SECONDS);
                    }
                }
            }
            catch(InterruptedException ex)
            {
                if(interruptible)
                {
                    throw new com.zeroc.Ice.OperationInterruptedException();
                }
            }

            //
            // NOTE: at this point destroy() can't be interrupted
            // anymore. The calls below are therefore guaranteed to be
            // called once.
            //

            for(com.zeroc.Ice.ObjectFactory f : _objectFactoryMap.values())
            {
                f.destroy();
            }
            _objectFactoryMap.clear();

            if(_routerManager != null)
            {
                _routerManager.destroy();
            }

            if(_locatorManager != null)
            {
                _locatorManager.destroy();
            }

            if(_endpointFactoryManager != null)
            {
                _endpointFactoryManager.destroy();
            }

            if(_initData.properties.getPropertyAsInt("Ice.Warn.UnusedProperties") > 0)
            {
                java.util.List<String> unusedProperties =
                    ((com.zeroc.Ice.PropertiesI)_initData.properties).getUnusedProperties();
                if(unusedProperties.size() != 0)
                {
                    StringBuilder message = new StringBuilder("The following properties were set but never read:");
                    for(String p : unusedProperties)
                    {
                        message.append("\n    ");
                        message.append(p);
                    }
                    _initData.logger.warning(message.toString());
                }
            }

            //
            // Destroy last so that a Logger plugin can receive all log/traces before its destruction.
            //
            if(_pluginManager != null)
            {
                _pluginManager.destroy();
            }

            synchronized(this)
            {
                _objectAdapterFactory = null;
                _outgoingConnectionFactory = null;
                _retryQueue = null;

                _serverThreadPool = null;
                _clientThreadPool = null;
                _endpointHostResolver = null;
                _timer = null;

                _referenceFactory = null;
                _requestHandlerFactory = null;
                _proxyFactory = null;
                _routerManager = null;
                _locatorManager = null;
                _endpointFactoryManager = null;

                _pluginManager = null;

                _adminAdapter = null;
                _adminFacets.clear();

                _queueExecutor = null;
                _queueExecutorService = null;

                _typeToClassMap.clear();

                _state = StateDestroyed;
                notifyAll();
            }
        }
        finally
        {
            synchronized(this)
            {
                if(_state == StateDestroyInProgress)
                {
                    assert(interruptible);
                    _state = StateActive;
                    notifyAll();
                }
            }
        }
    }

    public BufSizeWarnInfo getBufSizeWarn(short type)
    {
        synchronized(_setBufSizeWarn)
        {
            BufSizeWarnInfo info;
            if(!_setBufSizeWarn.containsKey(type))
            {
                info = new BufSizeWarnInfo();
                info.sndWarn = false;
                info.sndSize = -1;
                info.rcvWarn = false;
                info.rcvSize = -1;
                _setBufSizeWarn.put(type, info);
            }
            else
            {
                info = _setBufSizeWarn.get(type);
            }
            return info;
        }
    }

    public void setSndBufSizeWarn(short type, int size)
    {
        synchronized(_setBufSizeWarn)
        {
            BufSizeWarnInfo info = getBufSizeWarn(type);
            info.sndWarn = true;
            info.sndSize = size;
            _setBufSizeWarn.put(type, info);
        }
    }

    public void setRcvBufSizeWarn(short type, int size)
    {
        synchronized(_setBufSizeWarn)
        {
            BufSizeWarnInfo info = getBufSizeWarn(type);
            info.rcvWarn = true;
            info.rcvSize = size;
            _setBufSizeWarn.put(type, info);
        }
    }

    @SuppressWarnings("deprecation")
    public synchronized void addObjectFactory(final com.zeroc.Ice.ObjectFactory factory, String id)
    {
        //
        // Create a ValueFactory wrapper around the given ObjectFactory and register the wrapper
        // with the value factory manager. This may raise AlreadyRegisteredException.
        //
        _initData.valueFactoryManager.add(
            new com.zeroc.Ice.ValueFactory()
            {
                public com.zeroc.Ice.Value create(String id)
                {
                    return factory.create(id);
                }
            }, id);

        _objectFactoryMap.put(id, factory);
    }

    @SuppressWarnings("deprecation")
    public synchronized com.zeroc.Ice.ObjectFactory findObjectFactory(String id)
    {
        return _objectFactoryMap.get(id);
    }

    private void
    updateConnectionObservers()
    {
        try
        {
            assert(_outgoingConnectionFactory != null);
            _outgoingConnectionFactory.updateConnectionObservers();
            assert(_objectAdapterFactory != null);
            _objectAdapterFactory.updateConnectionObservers();
        }
        catch(com.zeroc.Ice.CommunicatorDestroyedException ex)
        {
        }
    }

    private void
    updateThreadObservers()
    {
        try
        {
            if(_clientThreadPool != null)
            {
                _clientThreadPool.updateObservers();
            }
            if(_serverThreadPool != null)
            {
                _serverThreadPool.updateObservers();
            }
            assert(_objectAdapterFactory != null);
            _objectAdapterFactory.updateThreadObservers();
            if(_endpointHostResolver != null)
            {
                _endpointHostResolver.updateObserver();
            }
            if(_timer != null)
            {
                _timer.updateObserver(_initData.observer);
            }
            if(_queueExecutor != null)
            {
                _queueExecutor.updateObserver(_initData.observer);
            }
        }
        catch(com.zeroc.Ice.CommunicatorDestroyedException ex)
        {
        }
    }

    private String[]
    validatePackages()
    {
        final String prefix = "Ice.Package.";
        java.util.Map<String, String> map = _initData.properties.getPropertiesForPrefix(prefix);
        java.util.List<String> packages = new java.util.ArrayList<>();
        for(java.util.Map.Entry<String, String> p : map.entrySet())
        {
            String key = p.getKey();
            String pkg = p.getValue();
            if(key.length() == prefix.length())
            {
                _initData.logger.warning("ignoring invalid property: " + key + "=" + pkg);
            }
            String module = key.substring(prefix.length());
            String className = pkg + "." + module + "._Marker";
            Class<?> cls = null;
            try
            {
                cls = findClass(className);
            }
            catch(java.lang.Exception ex)
            {
            }
            if(cls == null)
            {
                _initData.logger.warning("unable to validate package: " + key + "=" + pkg);
            }
            else
            {
                packages.add(pkg);
            }
        }

        String pkg = _initData.properties.getProperty("Ice.Default.Package");
        if(pkg.length() > 0)
        {
            packages.add(pkg);
        }
        return packages.toArray(new String[packages.size()]);
    }

    private synchronized void
    addAllAdminFacets()
    {
        java.util.Map<String, com.zeroc.Ice.Object> filteredFacets = new java.util.HashMap<>();
        for(java.util.Map.Entry<String, com.zeroc.Ice.Object> p : _adminFacets.entrySet())
        {
            if(_adminFacetFilter.isEmpty() || _adminFacetFilter.contains(p.getKey()))
            {
                _adminAdapter.addFacet(p.getValue(), _adminIdentity, p.getKey());
            }
            else
            {
                filteredFacets.put(p.getKey(), p.getValue());
            }
        }
        _adminFacets = filteredFacets;
    }

    private void
    setServerProcessProxy(com.zeroc.Ice.ObjectAdapter adminAdapter, com.zeroc.Ice.Identity adminIdentity)
    {
        com.zeroc.Ice.ObjectPrx admin = adminAdapter.createProxy(adminIdentity);
        com.zeroc.Ice.LocatorPrx locator = adminAdapter.getLocator();
        String serverId = _initData.properties.getProperty("Ice.Admin.ServerId");

        if(locator != null && !serverId.isEmpty())
        {
            com.zeroc.Ice.ProcessPrx process = com.zeroc.Ice.ProcessPrx.uncheckedCast(admin.ice_facet("Process"));
            try
            {
                //
                // Note that as soon as the process proxy is registered, the communicator might be
                // shutdown by a remote client and admin facets might start receiving calls.
                //
                locator.getRegistry().setServerProcessProxy(serverId, process);
            }
            catch(com.zeroc.Ice.ServerNotFoundException ex)
            {
                if(_traceLevels.location >= 1)
                {
                    StringBuilder s = new StringBuilder(128);
                    s.append("couldn't register server `");
                    s.append(serverId);
                    s.append("' with the locator registry:\n");
                    s.append("the server is not known to the locator registry");
                    _initData.logger.trace(_traceLevels.locationCat, s.toString());
                }

                throw new com.zeroc.Ice.InitializationException(
                    "Locator knows nothing about server `" + serverId + "'");
            }
            catch(com.zeroc.Ice.LocalException ex)
            {
                if(_traceLevels.location >= 1)
                {
                    StringBuilder s = new StringBuilder(128);
                    s.append("couldn't register server `");
                    s.append(serverId);
                    s.append("' with the locator registry:\n");
                    s.append(ex.toString());
                    _initData.logger.trace(_traceLevels.locationCat, s.toString());
                }
                throw ex;
            }

            if(_traceLevels.location >= 1)
            {
                StringBuilder s = new StringBuilder(128);
                s.append("registered server `");
                s.append(serverId);
                s.append("' with the locator registry");
                _initData.logger.trace(_traceLevels.locationCat, s.toString());
            }
        }
    }

    private NetworkProxy createNetworkProxy(com.zeroc.Ice.Properties properties, int protocolSupport)
    {
        String proxyHost;

        proxyHost = properties.getProperty("Ice.SOCKSProxyHost");
        if(!proxyHost.isEmpty())
        {
            if(protocolSupport == Network.EnableIPv6)
            {
                throw new com.zeroc.Ice.InitializationException("IPv6 only is not supported with SOCKS4 proxies");
            }
            int proxyPort = properties.getPropertyAsIntWithDefault("Ice.SOCKSProxyPort", 1080);
            return new SOCKSNetworkProxy(proxyHost, proxyPort);
        }

        proxyHost = properties.getProperty("Ice.HTTPProxyHost");
        if(!proxyHost.isEmpty())
        {
            return new HTTPNetworkProxy(proxyHost, properties.getPropertyAsIntWithDefault("Ice.HTTPProxyPort", 1080));
        }

        return null;
    }

    private static final int StateActive = 0;
    private static final int StateDestroyInProgress = 1;
    private static final int StateDestroyed = 2;
    private int _state;

    private final com.zeroc.Ice.InitializationData _initData; // Immutable, not reset by destroy().
    private final TraceLevels _traceLevels; // Immutable, not reset by destroy().
    private final DefaultsAndOverrides _defaultsAndOverrides; // Immutable, not reset by destroy().
    private final int _messageSizeMax; // Immutable, not reset by destroy().
    private final int _batchAutoFlushSize; // Immutable, not reset by destroy().
    private final com.zeroc.Ice.ToStringMode _toStringMode; // Immutable, not reset by destroy().
    private final int _cacheMessageBuffers; // Immutable, not reset by destroy().
    private final ACMConfig _clientACM; // Immutable, not reset by destroy().
    private final ACMConfig _serverACM; // Immutable, not reset by destroy().
    private final com.zeroc.Ice.ImplicitContextI _implicitContext;
    private RouterManager _routerManager;
    private LocatorManager _locatorManager;
    private ReferenceFactory _referenceFactory;
    private RequestHandlerFactory _requestHandlerFactory;
    private ProxyFactory _proxyFactory;
    private OutgoingConnectionFactory _outgoingConnectionFactory;
    private ObjectAdapterFactory _objectAdapterFactory;
    private int _protocolSupport;
    private boolean _preferIPv6;
    private NetworkProxy _networkProxy;
    private ThreadPool _clientThreadPool;
    private ThreadPool _serverThreadPool;
    private EndpointHostResolver _endpointHostResolver;
    private RetryQueue _retryQueue;
    private Timer _timer;
    private EndpointFactoryManager _endpointFactoryManager;
    private com.zeroc.Ice.PluginManager _pluginManager;

    private boolean _adminEnabled = false;
    private com.zeroc.Ice.ObjectAdapter _adminAdapter;
    private java.util.Map<String, com.zeroc.Ice.Object> _adminFacets = new java.util.HashMap<>();
    private java.util.Set<String> _adminFacetFilter = new java.util.HashSet<>();
    private com.zeroc.Ice.Identity _adminIdentity;
    private java.util.Map<Short, BufSizeWarnInfo> _setBufSizeWarn = new java.util.HashMap<>();

    private java.util.Map<String, String> _typeToClassMap = new java.util.HashMap<>();
    final private String[] _packages;
    final private boolean _useApplicationClassLoader;

    private static boolean _oneOffDone = false;
    private QueueExecutorService _queueExecutorService;
    private QueueExecutor _queueExecutor;

    @SuppressWarnings("deprecation")
    private java.util.HashMap<String, com.zeroc.Ice.ObjectFactory> _objectFactoryMap = new java.util.HashMap<>();
}
