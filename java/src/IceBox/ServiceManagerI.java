// **********************************************************************
//
// Copyright (c) 2002
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package IceBox;

public final class ServiceManagerI extends _ServiceManagerDisp
{
    public
    ServiceManagerI(Ice.Application server, String[] args)
    {
	_server = server;
        _logger = _server.communicator().getLogger();
        _argv = args;
    }

    public void
    shutdown(Ice.Current current)
    {
        _server.communicator().shutdown();
    }

    int
    run()
    {
        try
        {
            //
            // Create an object adapter. Services probably should NOT share
            // this object adapter, as the endpoint(s) for this object adapter
            // will most likely need to be firewalled for security reasons.
            //
            Ice.ObjectAdapter adapter =
                _server.communicator().createObjectAdapter("IceBox.ServiceManager");

	    Ice.Properties properties = _server.communicator().getProperties();

	    String identity = properties.getPropertyWithDefault("IceBox.ServiceManager.Identity", "ServiceManager");
            adapter.add(this, Ice.Util.stringToIdentity(identity));

            //
            // Load and start the services defined in the property set
            // with the prefix "IceBox.Service.". These properties should
            // have the following format:
            //
            // IceBox.Service.Foo=Package.Foo [args]
            //
            final String prefix = "IceBox.Service.";
	    java.util.Map services = properties.getPropertiesForPrefix(prefix);
	    java.util.Iterator p = services.entrySet().iterator();
	    while(p.hasNext())
	    {
		java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
		String name = ((String)entry.getKey()).substring(prefix.length());
		String value = (String)entry.getValue();

                //
                // Separate the entry point from the arguments.
                //
                String className;
                String[] args;
                int pos = value.indexOf(' ');
                if(pos == -1)
                {
                    pos = value.indexOf('\t');
                }
                if(pos == -1)
                {
                    pos = value.indexOf('\n');
                }
                if(pos == -1)
                {
                    className = value;
                    args = new String[0];
                }
                else
                {
                    className = value.substring(0, pos);
                    args = value.substring(pos).trim().split("[ \t\n]+", pos);
                }

                start(name, className, args);
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
            // Start request dispatching after we've started the services.
            //
            adapter.activate();

            _server.shutdownOnInterrupt();
            _server.communicator().waitForShutdown();
	    _server.ignoreInterrupt();

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
        catch(Ice.RuntimeException ex)
        {
            java.io.StringWriter sw = new java.io.StringWriter();
            java.io.PrintWriter pw = new java.io.PrintWriter(sw);
            ex.printStackTrace(pw);
            pw.flush();
            _logger.error("ServiceManager: " + ex + "\n" + sw.toString());
            stopAll();
            return 1;
        }
        catch(Exception ex)
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
    start(String service, String className, String[] args)
        throws FailureException
    {
	//
	// Create the service property set from the service arguments
	// and the server arguments. The service property set will be
	// use to create a new communicator or we be added to the
	// shared communicator depending on the value of the
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

	String[] serviceArgs = new String[l.size()];
        l.toArray(serviceArgs);

        //
        // Instantiate the class.
        //
	ServiceInfo info = new ServiceInfo();
        try
        {
            Class c = Class.forName(className);
            java.lang.Object obj = c.newInstance();
            try
            {
                info.service = (ServiceBase)obj;
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
	    // If Ice.UseSharedCommunicator.<name> is defined create a
	    // communicator for the service. The communicator inherits
	    // from the shared communicator properties. If it's not
	    // defined, add the service properties to the shared
	    // commnunicator property set.
	    //
	    Ice.Properties properties = _server.communicator().getProperties();
	    if(properties.getPropertyAsInt("IceBox.UseSharedCommunicator." + service) > 0)
	    {
		Ice.Properties fileProperties = Ice.Util.createProperties(serviceArgs);
		properties.parseCommandLineOptions("", fileProperties.getCommandLineOptions());

		serviceArgs = properties.parseCommandLineOptions("Ice", serviceArgs);
		serviceArgs = properties.parseCommandLineOptions(service, serviceArgs);
	    }
	    else
	    {
		Ice.Properties serviceProperties = properties._clone();

		Ice.Properties fileProperties = Ice.Util.createProperties(serviceArgs);
		serviceProperties.parseCommandLineOptions("", fileProperties.getCommandLineOptions());

		serviceArgs = serviceProperties.parseCommandLineOptions("Ice", serviceArgs);
		serviceArgs = serviceProperties.parseCommandLineOptions(service, serviceArgs);

		info.communicator = Ice.Util.initializeWithProperties(new String[0], serviceProperties);
	    }
	
	    Ice.Communicator communicator = info.communicator != null ? info.communicator : _server.communicator();

	    try
	    {
	        //
		// IceBox::Service
		//
	        Service s = (Service)info.service;
	        info.dbEnv = null;
                s.start(service, communicator, serviceArgs);
	    }
	    catch(ClassCastException e)
	    {
	        //
		// IceBox::FreezeService
		//
		// Either open the database environment or if it has already been opened
		// retrieve it from the map.
		//
	        FreezeService fs = (FreezeService)info.service;

		info.dbEnv = Freeze.Util.initialize(communicator, 
						    properties.getProperty("IceBox.DBEnvName." + service));
		
                fs.start(service, communicator, serviceArgs, info.dbEnv);
	    }
            _services.put(service, info);
        }
	catch(Freeze.DBException ex)
	{
            FailureException e = new FailureException();
            e.reason = "ServiceManager: database exception while starting service " + service + ": " + ex;
            e.initCause(ex);
            throw e;
	}
        catch(FailureException ex)
        {
            throw ex;
        }
        catch(Exception ex)
        {
            FailureException e = new FailureException();
            e.reason = "ServiceManager: exception while starting service " + service + ": " + ex;
            e.initCause(ex);
            throw e;
        }
    }

    private void
    stop(String service)
        throws FailureException
    {
        ServiceInfo info = (ServiceInfo)_services.remove(service);
        assert(info != null);

        try
        {
            info.service.stop();

	    if(info.dbEnv != null)
	    {
		info.dbEnv.close();
	    }
        }
	catch(Freeze.DBException ex)
	{
	    if(info.communicator != null)
	    {
		try
		{
		    info.communicator.destroy();
		}
		catch(Exception e)
		{
		}
	    }

            FailureException e = new FailureException();
            e.reason = "ServiceManager: database exception in stop for service " + service + ": " + ex;
            e.initCause(ex);
            throw e;
	}
        catch(Exception ex)
        {
	    if(info.communicator != null)
	    {
		try
		{
		    info.communicator.destroy();
		}
		catch(Exception e)
		{
		}
	    }

            FailureException e = new FailureException();
            e.reason = "ServiceManager: exception in stop for service " + service + ": " + ex;
            e.initCause(ex);
            throw e;
        }

	if(info.communicator != null)
	{
	    try
	    {
		info.communicator.destroy();
	    }
	    catch(Exception ex)
	    {
	    }
	}
    }

    private void
    stopAll()
    {
	java.util.Iterator r;
        do
        {
	    r = _services.entrySet().iterator();
            java.util.Map.Entry e = (java.util.Map.Entry)r.next();
            String name = (String)e.getKey();
            try
            {
		stop(name);
            }
            catch(Exception ex)
            {
                java.io.StringWriter sw = new java.io.StringWriter();
                java.io.PrintWriter pw = new java.io.PrintWriter(sw);
                ex.printStackTrace(pw);
                pw.flush();
                _logger.error("ServiceManager: exception in stop for service " + name + "\n" + sw.toString());
            }
        }
	while(r.hasNext());

        _services.clear();
    }

    class ServiceInfo
    {
        public ServiceBase service;
	public Ice.Communicator communicator = null;
        Freeze.DBEnvironment dbEnv;
    }

    private Ice.Application _server;
    private Ice.Logger _logger;
    private String[] _argv; // Filtered server argument vector
    private java.util.HashMap _services = new java.util.HashMap();
    private java.util.HashMap _dbEnvs = new java.util.HashMap();
}
