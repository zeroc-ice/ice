// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceBox;

//
// NOTE: the class isn't final on purpose to allow users to eventually
// extend it.
//
public class ServiceManagerI extends _ServiceManagerDisp
{
    public
    ServiceManagerI(Ice.Application server, String[] args)
    {
        _server = server;
        _logger = _server.communicator().getLogger();
        _argv = args;
        _traceServiceObserver =
            _server.communicator().getProperties().getPropertyAsInt("IceBox.Trace.ServiceObserver");
    }

    public java.util.Map
    getSliceChecksums(Ice.Current current)
    {
        return SliceChecksums.checksums;
    }

    public void
    startService(String name, Ice.Current current)
        throws AlreadyStartedException, NoSuchServiceException
    {
        ServiceInfo info = null;
        synchronized(this)
        {
            //
            // Search would be more efficient if services were contained in
            // a map, but order is required for shutdown.
            //
            java.util.Iterator<ServiceInfo> p = _services.iterator();
            while(p.hasNext())
            {
                ServiceInfo in = p.next();
                if(in.name.equals(name))
                {
                    if(in.status == StatusStarted)
                    {
                        throw new AlreadyStartedException();
                    }
                    in.status = StatusStarting;
                    info = (ServiceInfo)in.clone();
                    break;
                }
            }
            if(info == null)
            {
                throw new NoSuchServiceException();
            }
            _pendingStatusChanges = true;
        }

        boolean started = false;
        try
        {
            info.service.start(name, info.communicator == null ? _server.communicator() : info.communicator,
                               info.args);
            started = true;
        }
        catch(java.lang.Exception e)
        {
            java.io.StringWriter sw = new java.io.StringWriter();
            java.io.PrintWriter pw = new java.io.PrintWriter(sw);
            e.printStackTrace(pw);
            pw.flush();
            _logger.warning("ServiceManager: exception in start for service " + info.name + "\n" +
                            sw.toString());
        }

        java.util.Set<ServiceObserverPrx> observers = null;
        synchronized(this)
        {
            java.util.Iterator<ServiceInfo> p = _services.iterator();
            while(p.hasNext())
            {
                ServiceInfo in = p.next();
                if(in.name.equals(name))
                {
                    if(started)
                    {
                        in.status = StatusStarted;

                        java.util.List<String> services = new java.util.ArrayList<String>();
                        services.add(name);
                        servicesStarted(services, _observers);                        
                    }
                    else
                    {
                        in.status = StatusStopped;
                    }
                    break;
                }
            }
            _pendingStatusChanges = false;
            notifyAll();
        }
    }

    public void
    stopService(String name, Ice.Current current)
        throws AlreadyStoppedException, NoSuchServiceException
    {
        ServiceInfo info = null;
        synchronized(this)
        {
            //
            // Search would be more efficient if services were contained in
            // a map, but order is required for shutdown.
            //
            java.util.Iterator<ServiceInfo> p = _services.iterator();
            while(p.hasNext())
            {
                ServiceInfo in = p.next();
                if(in.name.equals(name))
                {
                    if(in.status == StatusStopped)
                    {
                        throw new AlreadyStoppedException();
                    }
                    in.status = StatusStopping;
                    info = (ServiceInfo)in.clone();
                    break;
                }
            }
            if(info == null)
            {
                throw new NoSuchServiceException();
            }
            _pendingStatusChanges = true;
        }

        boolean stopped = false;
        try
        {
            info.service.stop();
            stopped = true;
        }
        catch(java.lang.Exception e)
        {
            java.io.StringWriter sw = new java.io.StringWriter();
            java.io.PrintWriter pw = new java.io.PrintWriter(sw);
            e.printStackTrace(pw);
            pw.flush();
            _logger.warning("ServiceManager: exception in stop for service " + info.name + "\n" +
                            sw.toString());
        }

        synchronized(this)
        {
            java.util.Iterator<ServiceInfo> p = _services.iterator();
            while(p.hasNext())
            {
                ServiceInfo in = p.next();
                if(in.name.equals(name))
                {
                    if(stopped)
                    {
                        in.status = StatusStopped;

                        java.util.List<String> services = new java.util.ArrayList<String>();
                        services.add(name);
                        servicesStopped(services, _observers);
                    }
                    else
                    {
                        in.status = StatusStarted;
                    }
                    break;
                }
            }
            _pendingStatusChanges = false;
            notifyAll();
        }
    }

    public void
    addObserver(final ServiceObserverPrx observer, Ice.Current current)
    {
        java.util.List<String> activeServices = new java.util.LinkedList<String>();

        //
        // Null observers and duplicate registrations are ignored
        //

        synchronized(this)
        {
            if(observer != null && _observers.add(observer))
            {
                if(_traceServiceObserver >= 1)
                {
                    _logger.trace("IceBox.ServiceObserver",
                                  "Added service observer " + _server.communicator().proxyToString(observer));
                }


                for(ServiceInfo info: _services)
                {
                    if(info.status == StatusStarted)
                    {
                        activeServices.add(info.name);
                    }
                }

            }
        }

        if(activeServices.size() > 0)
        {
            AMI_ServiceObserver_servicesStarted cb = new AMI_ServiceObserver_servicesStarted()
                {
                    public void ice_response()
                    {
                        // ok, success
                    }

                    public void ice_exception(Ice.LocalException ex)
                    {
                        //
                        // Drop this observer
                        //
                        removeObserver(observer, ex);
                    }
                };

            observer.servicesStarted_async(cb, activeServices.toArray(new String[0]));
        }
    }


    public void
    shutdown(Ice.Current current)
    {
        _server.communicator().shutdown();
    }

    public int
    run()
    {
        try
        {
            Ice.Properties properties = _server.communicator().getProperties();

            //
            // Create an object adapter. Services probably should NOT share
            // this object adapter, as the endpoint(s) for this object adapter
            // will most likely need to be firewalled for security reasons.
            //
            Ice.ObjectAdapter adapter = null;
            if(!properties.getProperty("IceBox.ServiceManager.Endpoints").equals(""))
            {
                adapter = _server.communicator().createObjectAdapter("IceBox.ServiceManager");

                Ice.Identity identity = new Ice.Identity();
                identity.category = properties.getPropertyWithDefault("IceBox.InstanceName", "IceBox");
                identity.name = "ServiceManager";
                adapter.add(this, identity);
            }

            //
            // Load and start the services defined in the property set
            // with the prefix "IceBox.Service.". These properties should
            // have the following format:
            //
            // IceBox.Service.Foo=Package.Foo [args]
            //
            // We load the services specified in IceBox.LoadOrder first,
            // then load any remaining services.
            //
            final String prefix = "IceBox.Service.";
            java.util.Map<String, String> services = properties.getPropertiesForPrefix(prefix);
            String[] loadOrder = properties.getPropertyAsList("IceBox.LoadOrder");
            for(int i = 0; i < loadOrder.length; ++i)
            {
                if(loadOrder[i].length() > 0)
                {
                    String key = prefix + loadOrder[i];
                    String value = services.get(key);
                    if(value == null)
                    {
                        FailureException ex = new FailureException();
                        ex.reason = "ServiceManager: no service definition for `" + loadOrder[i] + "'";
                        throw ex;
                    }
                    load(loadOrder[i], value);
                    services.remove(key);
                }
            }
            java.util.Iterator<java.util.Map.Entry<String, String> > p = services.entrySet().iterator();
            while(p.hasNext())
            {
                java.util.Map.Entry<String, String> entry = p.next();
                String name = entry.getKey().substring(prefix.length());
                String value = entry.getValue();
                load(name, value);
            }

            //
            // We may want to notify external scripts that the services
            // have started. This is done by defining the property:
            //
            // IceBox.PrintServicesReady=bundleName
            //
            // Where bundleName is whatever you choose to call this set of
            // services. It will be echoed back as "bundleName ready".
            //
            // This must be done after start() has been invoked on the
            // services.
            //
            String bundleName = properties.getProperty("IceBox.PrintServicesReady");
            if(bundleName.length() > 0)
            {
                System.out.println(bundleName + " ready");
            }

            //
            // Don't move after the adapter activation. This allows
            // applications to wait for the service manager to be
            // reachable before sending a signal to shutdown the
            // IceBox.
            //
            _server.shutdownOnInterrupt();


            //
            // Register "this" as a facet to the Admin object and
            // create Admin object
            //
            try
            {
                _server.communicator().addAdminFacet(this, "IceBox.ServiceManager");

                //
                // Add a Properties facet for each service
                //
                for(ServiceInfo info: _services)
                {
                    Ice.Communicator communicator = info.communicator != null ? info.communicator : _sharedCommunicator;
                    _server.communicator().addAdminFacet(new PropertiesAdminI(communicator.getProperties()),
                                                         "IceBox.Service." + info.name + ".Properties");
                }

                _server.communicator().getAdmin();
            }
            catch(Ice.ObjectAdapterDeactivatedException ex)
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
                catch(Ice.ObjectAdapterDeactivatedException ex)
                {
                    //
                    // Expected if the communicator has been shutdown.
                    //
                }
            }

            _server.communicator().waitForShutdown();
            _server.defaultInterrupt();

            //
            // Invoke stop() on the services.
            //
            stopAll();
        }
        catch(FailureException ex)
        {
            java.io.StringWriter sw = new java.io.StringWriter();
            java.io.PrintWriter pw = new java.io.PrintWriter(sw);
            pw.println(ex.reason);
            ex.printStackTrace(pw);
            pw.flush();
            _logger.error(sw.toString());
            stopAll();
            return 1;
        }
        catch(Ice.LocalException ex)
        {
            java.io.StringWriter sw = new java.io.StringWriter();
            java.io.PrintWriter pw = new java.io.PrintWriter(sw);
            ex.printStackTrace(pw);
            pw.flush();
            _logger.error("ServiceManager: " + ex + "\n" + sw.toString());
            stopAll();
            return 1;
        }
        catch(java.lang.Exception ex)
        {
            java.io.StringWriter sw = new java.io.StringWriter();
            java.io.PrintWriter pw = new java.io.PrintWriter(sw);
            ex.printStackTrace(pw);
            pw.flush();
            _logger.error("ServiceManager: unknown exception\n" + sw.toString());
            stopAll();
            return 1;
        }

        return 0;
    }


    private void
    load(String name, String value)
        throws FailureException
    {
        //
        // Separate the entry point from the arguments.
        //
        String className;
        String[] args;
        int pos = IceUtilInternal.StringUtil.findFirstOf(value, " \t\n");
        if(pos == -1)
        {
            className = value;
            args = new String[0];
        }
        else
        {
            className = value.substring(0, pos);
            try
            {
                args = IceUtilInternal.Options.split(value.substring(pos));
            }
            catch(IceUtilInternal.Options.BadQuote ex)
            {
                FailureException e = new FailureException();
                e.reason = "ServiceManager: invalid arguments for service `" + name + "':\n" + ex.toString();
                throw e;
            }
        }

        start(name, className, args);
    }

    synchronized private void
    start(String service, String className, String[] args)
        throws FailureException
    {
        //
        // Create the service property set from the service arguments
        // and the server arguments. The service property set will be
        // used to create a new communicator, or will be added to the
        // shared communicator, depending on the value of the
        // IceBox.UseSharedCommunicator property.
        //
        java.util.List<String> l = new java.util.ArrayList<String>();
        for(int j = 0; j < args.length; j++)
        {
            l.add(args[j]);
        }
        for(int j = 0; j < _argv.length; j++)
        {
            if(_argv[j].startsWith("--" + service + "."))
            {
                l.add(_argv[j]);
            }
        }

        Ice.StringSeqHolder serviceArgs = new Ice.StringSeqHolder();
        serviceArgs.value = (String[])l.toArray(new String[0]);

        //
        // Instantiate the class.
        //
        ServiceInfo info = new ServiceInfo();
        info.name = service;
        info.status = StatusStopped;
        try
        {
            Class c = Class.forName(className);
            java.lang.Object obj = c.newInstance();
            try
            {
                info.service = (Service)obj;
            }
            catch(ClassCastException ex)
            {
                FailureException e = new FailureException();
                e.reason = "ServiceManager: class " + className + " does not implement IceBox.Service";
                throw e;
            }
        }
        catch(ClassNotFoundException ex)
        {
            FailureException e = new FailureException();
            e.reason = "ServiceManager: class " + className + " not found";
            e.initCause(ex);
            throw e;
        }
        catch(IllegalAccessException ex)
        {
            FailureException e = new FailureException();
            e.reason = "ServiceManager: unable to access default constructor in class " + className;
            e.initCause(ex);
            throw e;
        }
        catch(InstantiationException ex)
        {
            FailureException e = new FailureException();
            e.reason = "ServiceManager: unable to instantiate class " + className;
            e.initCause(ex);
            throw e;
        }

        //
        // Invoke Service::start().
        //
        try
        {
            //
            // If Ice.UseSharedCommunicator.<name> is defined, create a
            // communicator for the service. The communicator inherits
            // from the shared communicator properties. If it's not
            // defined, add the service properties to the shared
            // commnunicator property set.
            //
            Ice.Communicator communicator;
            if(_server.communicator().getProperties().getPropertyAsInt("IceBox.UseSharedCommunicator." + service) > 0)
            {
                if(_sharedCommunicator == null)
                {
                    _sharedCommunicator = createCommunicator("", new Ice.StringSeqHolder());
                }
                communicator = _sharedCommunicator;

                Ice.Properties properties = _sharedCommunicator.getProperties();
                Ice.Properties svcProperties = Ice.Util.createProperties(serviceArgs, properties);

                //
                // Erase properties in 'properties'
                //
                java.util.Map<String, String> allProps = properties.getPropertiesForPrefix("");
                java.util.Iterator<String> p = allProps.keySet().iterator();
                while(p.hasNext())
                {
                    String key = p.next();
                    if(svcProperties.getProperty(key).length() == 0)
                    {
                        properties.setProperty(key, "");
                    }
                }

                //
                // Add the service properties to the shared communicator properties.
                //
                java.util.Iterator<java.util.Map.Entry<String, String> > q =
                    svcProperties.getPropertiesForPrefix("").entrySet().iterator();
                while(q.hasNext())
                {
                    java.util.Map.Entry<String, String> entry = q.next();
                    properties.setProperty(entry.getKey(), entry.getValue());
                }

                //
                // Parse <service>.* command line options
                // (the Ice command line options were parse by the createProperties above)
                //
                serviceArgs.value = properties.parseCommandLineOptions(service, serviceArgs.value);
            }
            else
            {
                info.communicator = createCommunicator(service, serviceArgs);
                communicator = info.communicator;
            }

            try
            {
                info.args = serviceArgs.value;
                info.service.start(service, communicator, info.args);
                info.status = StatusStarted;

                //
                // There is no need to notify the observers since the 'start all'
                // (that indirectly calls this method) occurs before the creation of
                // the Server Admin object, and before the activation of the main
                // object adapter (so before any observer can be registered)
                //
            }
            catch(Throwable ex)
            {
                if(info.communicator != null)
                {
                    try
                    {
                        info.communicator.shutdown();
                        info.communicator.waitForShutdown();
                    }
                    catch(Ice.CommunicatorDestroyedException e)
                    {
                        //
                        // Ignore, the service might have already destroyed
                        // the communicator for its own reasons.
                        //
                    }
                    catch(java.lang.Exception e)
                    {
                        java.io.StringWriter sw = new java.io.StringWriter();
                        java.io.PrintWriter pw = new java.io.PrintWriter(sw);
                        e.printStackTrace(pw);
                        pw.flush();
                        _logger.warning("ServiceManager: exception in shutting down communicator for service "
                                        + service + "\n" + sw.toString());
                    }

                    try
                    {
                        info.communicator.destroy();
                    }
                    catch(java.lang.Exception e)
                    {
                        java.io.StringWriter sw = new java.io.StringWriter();
                        java.io.PrintWriter pw = new java.io.PrintWriter(sw);
                        e.printStackTrace(pw);
                        pw.flush();
                        _logger.warning("ServiceManager: exception in destroying communciator for service"
                                        + service + "\n" + sw.toString());
                    }
                }
                throw ex;
            }

            _services.add(info);
        }
        catch(FailureException ex)
        {
            throw ex;
        }
        catch(Throwable ex)
        {
            FailureException e = new FailureException();
            e.reason = "ServiceManager: exception while starting service " + service + ": " + ex;
            e.initCause(ex);
            throw e;
        }
    }

    private synchronized void
    stopAll()
    {
        //
        // First wait for any active startService/stopService calls to complete.
        //
        while(_pendingStatusChanges)
        {
            try
            {
                wait();
            }
            catch(java.lang.InterruptedException ex)
            {
            }
        }

        //
        // For each service, we call stop on the service and flush its database environment to
        // the disk. Services are stopped in the reverse order of the order they were started.
        //
        java.util.List<String> stoppedServices = new java.util.ArrayList<String>();
        java.util.ListIterator<ServiceInfo> p = _services.listIterator(_services.size());
        while(p.hasPrevious())
        {
            ServiceInfo info = p.previous();
            if(info.status == StatusStarted)
            {
                try
                {
                    info.service.stop();
                    info.status = StatusStopped;
                    stoppedServices.add(info.name);
                }
                catch(java.lang.Exception e)
                {
                    java.io.StringWriter sw = new java.io.StringWriter();
                    java.io.PrintWriter pw = new java.io.PrintWriter(sw);
                    e.printStackTrace(pw);
                    pw.flush();
                    _logger.warning("ServiceManager: exception in stop for service " + info.name + "\n" +
                                    sw.toString());
                }
            }

            try
            {
                _server.communicator().removeAdminFacet("IceBox.Service." + info.name + ".Properties");
            }
            catch(Ice.LocalException e)
            {
                // Ignored
            }

            if(info.communicator != null)
            {
                try
                {
                    info.communicator.shutdown();
                    info.communicator.waitForShutdown();
                }
                catch(Ice.CommunicatorDestroyedException e)
                {
                    //
                    // Ignore, the service might have already destroyed
                    // the communicator for its own reasons.
                    //
                }
                catch(java.lang.Exception e)
                {
                    java.io.StringWriter sw = new java.io.StringWriter();
                    java.io.PrintWriter pw = new java.io.PrintWriter(sw);
                    e.printStackTrace(pw);
                    pw.flush();
                    _logger.warning("ServiceManager: exception in stop for service " + info.name + "\n" +
                                    sw.toString());
                }

                try
                {
                    info.communicator.destroy();
                }
                catch(java.lang.Exception e)
                {
                    java.io.StringWriter sw = new java.io.StringWriter();
                    java.io.PrintWriter pw = new java.io.PrintWriter(sw);
                    e.printStackTrace(pw);
                    pw.flush();
                    _logger.warning("ServiceManager: exception in stop for service " + info.name + "\n" +
                                    sw.toString());
                }
            }
        }

        if(_sharedCommunicator != null)
        {
            try
            {
                _sharedCommunicator.destroy();
            }
            catch(Exception e)
            {
                java.io.StringWriter sw = new java.io.StringWriter();
                java.io.PrintWriter pw = new java.io.PrintWriter(sw);
                e.printStackTrace(pw);
                pw.flush();
                _logger.warning("ServiceManager: unknown exception while destroying shared communicator:\n" +
                                sw.toString());
            }
            _sharedCommunicator = null;
        }

        _services.clear();
        servicesStopped(stoppedServices, _observers);
    }

    private void
    servicesStarted(java.util.List<String> services, java.util.Set<ServiceObserverPrx> observers)
    {
        if(services.size() > 0)
        {
            String[] servicesArray = services.toArray(new String[0]);

            for(final ServiceObserverPrx observer: observers)
            {
                AMI_ServiceObserver_servicesStarted cb = new AMI_ServiceObserver_servicesStarted()
                    {
                        public void ice_response()
                        {
                            // ok, success
                        }

                        public void ice_exception(Ice.LocalException ex)
                        {
                            //
                            // Drop this observer
                            //
                            removeObserver(observer, ex);
                        }
                    };

                observer.servicesStarted_async(cb, servicesArray);
            }
        }
    }

    private void
    servicesStopped(java.util.List<String> services, java.util.Set<ServiceObserverPrx> observers)
    {
        if(services.size() > 0)
        {
            String[] servicesArray = services.toArray(new String[0]);

            for(final ServiceObserverPrx observer: observers)
            {
                AMI_ServiceObserver_servicesStopped cb = new AMI_ServiceObserver_servicesStopped()
                    {
                        public void ice_response()
                        {
                            // ok, success
                        }

                        public void ice_exception(Ice.LocalException ex)
                        {
                            //
                            // Drop this observer
                            //
                            removeObserver(observer, ex);
                        }
                    };

                observer.servicesStopped_async(cb, servicesArray);
            }
        }
    }


    private synchronized void
    removeObserver(ServiceObserverPrx observer, Ice.LocalException ex)
    {
        if(_observers.remove(observer))
        {
            observerRemoved(observer, ex);
        }
    }

    private void
    observerRemoved(ServiceObserverPrx observer, RuntimeException ex)
    {
        if(_traceServiceObserver >= 1)
        {
            //
            // CommunicatorDestroyedException may occur during shutdown. The observer notification has
            // been sent, but the communicator was destroyed before the reply was received. We do not
            // log a message for this exception.
            //
            if(!(ex instanceof Ice.CommunicatorDestroyedException))
            {
                _logger.trace("IceBox.ServiceObserver",
                              "Removed service observer " + _server.communicator().proxyToString(observer)
                              + "\nafter catching " + ex.toString());
            }
        }
    }

    public final static int StatusStopping = 0;
    public final static int StatusStopped = 1;
    public final static int StatusStarting = 2;
    public final static int StatusStarted = 3;

    static class ServiceInfo implements Cloneable
    {
        public Object clone()
        {
            Object o = null;
            try
            {
                o = super.clone();
            }
            catch(CloneNotSupportedException ex)
            {
            }
            return o;
        }

        public String name;
        public Service service;
        public Ice.Communicator communicator = null;
        public int status;
        public String[] args;
    }

    static class PropertiesAdminI extends Ice._PropertiesAdminDisp
    {
        PropertiesAdminI(Ice.Properties properties)
        {
            _properties = properties;
        }

        public String
        getProperty(String name, Ice.Current current)
        {
            return _properties.getProperty(name);
        }

        public java.util.TreeMap<String, String>
        getPropertiesForPrefix(String name, Ice.Current current)
        {
            return new java.util.TreeMap<String, String>(_properties.getPropertiesForPrefix(name));
        }

        private final Ice.Properties _properties;
    }

    private Ice.Communicator
    createCommunicator(String service, Ice.StringSeqHolder args)
    {
        Ice.Properties communicatorProperties = _server.communicator().getProperties();

        //
        // Create the service properties. We use the communicator properties as the default
        // properties if IceBox.InheritProperties is set.
        //
        Ice.Properties properties;
        if(communicatorProperties.getPropertyAsInt("IceBox.InheritProperties") > 0)
        {
            properties = communicatorProperties._clone();
            properties.setProperty("Ice.Admin.Endpoints", ""); // Inherit all except Ice.Admin.Endpoints!
        }
        else
        {
            properties = Ice.Util.createProperties();
        }

        //
        // Set the default program name for the service properties. By default it's
        // the IceBox program name + "-" + the service name, or just the IceBox
        // program name if we're creating the shared communicator (service == "").
        //
        String programName = communicatorProperties.getProperty("Ice.ProgramName");
        if(service.length() == 0)
        {
            if(programName.length() == 0)
            {
                properties.setProperty("Ice.ProgramName", "SharedCommunicator");
            }
            else
            {
                properties.setProperty("Ice.ProgramName", programName + "-SharedCommunicator");
            }
        }
        else
        {
            if(programName.length() == 0)
            {
                properties.setProperty("Ice.ProgramName", service);
            }
            else
            {
                properties.setProperty("Ice.ProgramName", programName + "-" + service);
            }
        }

        if(args.value != null && args.value.length > 0)
        {
            //
            // Create the service properties with the given service arguments. This should
            // read the service config file if it's specified with --Ice.Config.
            //
            properties = Ice.Util.createProperties(args, properties);

            if(service.length() > 0)
            {
                //
                // Next, parse the service "<service>.*" command line options (the Ice command
                // line options were parsed by the createProperties above)
                //
                args.value = properties.parseCommandLineOptions(service, args.value);
            }
        }

        //
        // Remaining command line options are passed to the communicator. This is
        // necessary for Ice plugin properties (e.g.: IceSSL).
        //
        Ice.InitializationData initData = new Ice.InitializationData();
        initData.properties = properties;
        if(args.value != null)
        {
            return Ice.Util.initialize(args, initData);
        }
        else
        {
            return Ice.Util.initialize(initData);
        }
    }

    private Ice.Application _server;
    private Ice.Communicator _sharedCommunicator;
    private Ice.Logger _logger;
    private String[] _argv; // Filtered server argument vector
    private java.util.List<ServiceInfo> _services = new java.util.LinkedList<ServiceInfo>();
    private boolean _pendingStatusChanges = false;

    java.util.HashSet<ServiceObserverPrx> _observers = new java.util.HashSet<ServiceObserverPrx>();
    int _traceServiceObserver = 0;
}
