// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
    }

    public java.util.Map
    getSliceChecksums(Ice.Current current)
    {
        return SliceChecksums.checksums;
    }

    synchronized public void
    startService(String name, Ice.Current current)
        throws AlreadyStartedException, NoSuchServiceException
    {
        //
        // Search would be more efficient if services were contained in
        // a map, but order is required for shutdown.
        //
        java.util.Iterator p = _services.iterator();
        while(p.hasNext())
        {
            ServiceInfo info = (ServiceInfo)p.next();
            if(info.name.equals(name))
            {
                if(info.active)
                {
                    throw new AlreadyStartedException();
                }

                try
                {
                    info.service.start(name, info.communicator == null ? _server.communicator() : info.communicator,
                                       info.args);
                    info.active = true;
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

                return;
            }
        }

        throw new NoSuchServiceException();
    }

    synchronized public void
    stopService(String name, Ice.Current current)
        throws AlreadyStoppedException, NoSuchServiceException
    {
        //
        // Search would be more efficient if services were contained in
        // a map, but order is required for shutdown.
        //
        java.util.Iterator p = _services.iterator();
        while(p.hasNext())
        {
            ServiceInfo info = (ServiceInfo)p.next();
            if(info.name.equals(name))
            {
                if(!info.active)
                {
                    throw new AlreadyStoppedException();
                }

                try
                {
                    info.service.stop();
                    info.active = false;
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

                return;
            }
        }

        throw new NoSuchServiceException();
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
            //
            // Create an object adapter. Services probably should NOT share
            // this object adapter, as the endpoint(s) for this object adapter
            // will most likely need to be firewalled for security reasons.
            //
            Ice.ObjectAdapter adapter = _server.communicator().createObjectAdapter("IceBox.ServiceManager");

            Ice.Properties properties = _server.communicator().getProperties();

            Ice.Identity identity = new Ice.Identity();
            identity.category = properties.getPropertyWithDefault("IceBox.InstanceName", "IceBox");
            identity.name = "ServiceManager";
            adapter.add(this, identity);

            //
            // Parse the IceBox.LoadOrder property.
            //
            String order = properties.getProperty("IceBox.LoadOrder");
            String[] loadOrder = null;
            if(order.length() > 0)
            {
                loadOrder = order.trim().split("[,\t ]+");
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
            java.util.Map services = properties.getPropertiesForPrefix(prefix);
            if(loadOrder != null)
            {
                for(int i = 0; i < loadOrder.length; ++i)
                {
                    if(loadOrder[i].length() > 0)
                    {
                        String key = prefix + loadOrder[i];
                        String value = (String)services.get(key);
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
            }
            java.util.Iterator p = services.entrySet().iterator();
            while(p.hasNext())
            {
                java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
                String name = ((String)entry.getKey()).substring(prefix.length());
                String value = (String)entry.getValue();
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
            // Start request dispatching after we've started the services.
            //
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
        int pos = IceUtil.StringUtil.findFirstOf(value, " \t\n");
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
                args = IceUtil.Options.split(value.substring(pos));
            }
            catch(IceUtil.Options.BadQuote ex)
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
        java.util.ArrayList l = new java.util.ArrayList();
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
            Ice.Properties properties = _server.communicator().getProperties();
            if(properties.getPropertyAsInt("IceBox.UseSharedCommunicator." + service) > 0)
            {
                Ice.Properties serviceProperties = Ice.Util.createProperties(serviceArgs, properties);

                //
                // Erase properties in 'properties'
                //
                java.util.Map allProps = properties.getPropertiesForPrefix("");
                java.util.Iterator p = allProps.keySet().iterator();
                while(p.hasNext())
                {
                    String key = (String)p.next();
                    if(serviceProperties.getProperty(key).length() == 0)
                    {
                        properties.setProperty(key, "");
                    }
                }
                
                //
                // Put all serviceProperties into 'properties'
                //
                properties.parseCommandLineOptions("", serviceProperties.getCommandLineOptions());
                
                //
                // Parse <service>.* command line options
                // (the Ice command line options were parse by the createProperties above)
                //
                serviceArgs.value = properties.parseCommandLineOptions(service, serviceArgs.value);
            }
            else
            {
                String name = properties.getProperty("Ice.ProgramName");
                Ice.Properties serviceProperties;
                if(properties.getPropertyAsInt("IceBox.InheritProperties") > 0)
                {
                    serviceProperties = Ice.Util.createProperties(serviceArgs, properties);
                }
                else
                {
                    serviceProperties = Ice.Util.createProperties(serviceArgs);
                }

                if(name.equals(serviceProperties.getProperty("Ice.ProgramName")))
                {
                    //
                    // If the service did not set its own program-name, and 
                    // the icebox program-name != service, append the service name to the 
                    // program name.
                    //
                    if(!name.equals(service))
                    {
                        name = name.length() == 0 ? service : name + "-" + service;
                    }
                    serviceProperties.setProperty("Ice.ProgramName", name);
                }
                
                //
                // Parse <service>.* command line options.
                // (the Ice command line options were parsed by the createProperties above)
                //
                serviceArgs.value = serviceProperties.parseCommandLineOptions(service, serviceArgs.value);

                //
                // Remaining command line options are passed to the
                // communicator with argc/argv. This is necessary for Ice
                // plugin properties (e.g.: IceSSL).
                //
                Ice.InitializationData initData = new Ice.InitializationData();
                initData.properties = serviceProperties;
                info.communicator = Ice.Util.initialize(serviceArgs, initData);
            }
        
            Ice.Communicator communicator = info.communicator != null ? info.communicator : _server.communicator();

            try
            {
                info.args = serviceArgs.value;
                info.service.start(service, communicator, info.args);
                info.active = true;
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

    synchronized private void
    stopAll()
    {
        //
        // First, for each service, we call stop on the service and flush its database environment to 
        // the disk. Services are stopped in the reverse order of the order they were started.
        //
        java.util.ListIterator p = _services.listIterator(_services.size());
        while(p.hasPrevious())
        {
            ServiceInfo info = (ServiceInfo)p.previous();
            if(info.active)
            {
                try
                {
                    info.service.stop();
                    info.active = false;
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

        _services.clear();
    }

    class ServiceInfo
    {
        public String name;
        public Service service;
        public Ice.Communicator communicator = null;
        public boolean active;
        public String[] args;
    }

    private Ice.Application _server;
    private Ice.Logger _logger;
    private String[] _argv; // Filtered server argument vector
    private java.util.List _services = new java.util.LinkedList();
}
