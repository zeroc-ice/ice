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
    ServiceManagerI(Ice.Communicator communicator, String[] args)
    {
        _communicator = communicator;
        _logger = _communicator.getLogger();
        _argv = args;

        Ice.Properties properties = _communicator.getProperties();
        _options = properties.getCommandLineOptions();
    }

    public void
    shutdown(Ice.Current current)
    {
        _communicator.shutdown();
    }

    int
    run()
    {
        try
        {
	    //
	    // Prefix the adapter name and object identity with the value
	    // of the IceBox.Name property.
	    //
	    Ice.Properties properties = _communicator.getProperties();
	    String namePrefix = properties.getProperty("IceBox.Name");
	    if(namePrefix.length() > 0)
	    {
		namePrefix += ".";
	    }

            //
            // Create an object adapter. Services probably should NOT share
            // this object adapter, as the endpoint(s) for this object adapter
            // will most likely need to be firewalled for security reasons.
            //
            Ice.ObjectAdapter adapter =
                _communicator.createObjectAdapterFromProperty(namePrefix + "ServiceManagerAdapter",
                                                              "IceBox.ServiceManager.Endpoints");

	    String identity = properties.getPropertyWithDefault("IceBox.ServiceManager.Identity", 
								namePrefix + "ServiceManager");
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

            _communicator.waitForShutdown();

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
        // We need to create a property set to pass to start()
        // The property set is populated from a number of sources.
        // The precedence order (from lowest to highest) is:
        //
        // 1. Properties defined in the server property set (e.g.,
        //    that were defined in the server's configuration file)
        // 2. Service arguments
        // 3. Server arguments
        //
        // We'll compose an array of arguments in the above order.
        //
        java.util.ArrayList l = new java.util.ArrayList();
        for(int j = 0; j < _options.length; j++)
        {
            if(_options[j].startsWith("--" + service + "."))
            {
                l.add(_options[j]);
            }
        }
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

        //
        // Create the service property set.
        //
        Ice.StringSeqHolder serviceArgs = new Ice.StringSeqHolder();
        serviceArgs.value = new String[l.size()];
        l.toArray(serviceArgs.value);
        Ice.Properties serviceProperties = Ice.Util.createProperties(serviceArgs);
        serviceArgs.value = serviceProperties.parseCommandLineOptions("Ice", serviceArgs.value);
        serviceArgs.value = serviceProperties.parseCommandLineOptions(service, serviceArgs.value);

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
	    try
	    {
	        //
		// IceBox::Service
		//
	        Service s = (Service)info.service;
	        info.dbEnvName = null;
                s.start(service, _communicator, serviceProperties, serviceArgs.value);
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

                Ice.Properties properties = _communicator.getProperties();
		String propName = "IceBox.DBEnvName." + service;
		info.dbEnvName = properties.getProperty(propName);

		DBEnvironmentInfo dbInfo = (DBEnvironmentInfo)_dbEnvs.get(info.dbEnvName);
		if(dbInfo == null)
		{
		    dbInfo = new DBEnvironmentInfo();
		    dbInfo.dbEnv = Freeze.Util.initialize(_communicator, info.dbEnvName);
		    dbInfo.openCount = 1;
		}
		else
		{
		    ++dbInfo.openCount;
		}
		_dbEnvs.put(info.dbEnvName, dbInfo);
		
                fs.start(service, _communicator, serviceProperties, serviceArgs.value, dbInfo.dbEnv);
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
	    try
	    {
	        FreezeService fs = (FreezeService)info.service;
		
	        DBEnvironmentInfo dbInfo = (DBEnvironmentInfo)_dbEnvs.get(info.dbEnvName);
		assert(dbInfo != null);
		if(--dbInfo.openCount == 0)
		{
		    dbInfo.dbEnv.close();
		    _dbEnvs.remove(info.dbEnvName);
		}
		else
		{
		    _dbEnvs.put(info.dbEnvName, dbInfo);
		}
	    }
	    catch(ClassCastException e)
	    {
	    }
        }
	catch(Freeze.DBException ex)
	{
            FailureException e = new FailureException();
            e.reason = "ServiceManager: database exception in stop for service " + service + ": " + ex;
            e.initCause(ex);
            throw e;
	}
        catch(Exception ex)
        {
            FailureException e = new FailureException();
            e.reason = "ServiceManager: exception in stop for service " + service + ": " + ex;
            e.initCause(ex);
            throw e;
        }
    }

    private void
    stopAll()
    {
        java.util.Iterator r = _services.entrySet().iterator();
        while(r.hasNext())
        {
            java.util.Map.Entry e = (java.util.Map.Entry)r.next();
            String name = (String)e.getKey();
            ServiceInfo info = (ServiceInfo)e.getValue();
            try
            {
                info.service.stop();
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
        _services.clear();
    }

    class ServiceInfo
    {
        public ServiceBase service;
	public String dbEnvName;
    }

    class DBEnvironmentInfo
    {
        Freeze.DBEnvironment dbEnv;
        public int openCount;
    }

    private Ice.Communicator _communicator;
    private Ice.Logger _logger;
    private String[] _argv; // Filtered server argument vector
    private String[] _options; // Server property set converted to command-line options
    private java.util.HashMap _services = new java.util.HashMap();
    private java.util.HashMap _dbEnvs = new java.util.HashMap();
}
