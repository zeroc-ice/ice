// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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

	    String identity = properties.getPropertyWithDefault("IceBox.ServiceManager.Identity", "ServiceManager");
	    adapter.add(this, Ice.Util.stringToIdentity(identity));

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
	    args = value.substring(pos).trim().split("[ \t\n]+", pos);
	}

	start(name, className, args);
    }

    private void
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
		Ice.Properties fileProperties = Ice.Util.createProperties(serviceArgs);
		properties.parseCommandLineOptions("", fileProperties.getCommandLineOptions());

		serviceArgs = properties.parseIceCommandLineOptions(serviceArgs);
		serviceArgs = properties.parseCommandLineOptions(service, serviceArgs);
	    }
	    else
	    {
		Ice.Properties serviceProperties = properties._clone();

		//
		// Initialize the Ice.ProgramName property with the name of this service.
		//
		String name = serviceProperties.getProperty("Ice.ProgramName");
		if(!name.equals(service))
		{
		    name = name.length() == 0 ? service : name + "-" + service;
		}

		Ice.Properties fileProperties = Ice.Util.createProperties(serviceArgs);
		serviceProperties.parseCommandLineOptions("", fileProperties.getCommandLineOptions());
		serviceProperties.setProperty("Ice.ProgramName", name);

		serviceArgs = serviceProperties.parseIceCommandLineOptions(serviceArgs);
		serviceArgs = serviceProperties.parseCommandLineOptions(service, serviceArgs);

		info.communicator = Ice.Util.initializeWithProperties(new String[0], serviceProperties);
	    }
	
	    Ice.Communicator communicator = info.communicator != null ? info.communicator : _server.communicator();

	    try
	    {
		info.service.start(service, communicator, serviceArgs);
	    }
	    //catch(Exception ex)
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

	    _services.put(service, info);
	}
	catch(FailureException ex)
	{
	    throw ex;
	}
	catch(Throwable ex)//java.lang.Exception ex)
	{
	    FailureException e = new FailureException();
	    e.reason = "ServiceManager: exception while starting service " + service + ": " + ex;
	    e.initCause(ex);
	    throw e;
	}
    }

    private void
    stopAll()
    {
	//
	// First, for each service, we call stop on the service and flush its database environment to 
	// the disk.
	//
	java.util.Iterator p = _services.entrySet().iterator();
	while(p.hasNext())
	{
	    java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
	    String name = (String)entry.getKey();
	    ServiceInfo info = (ServiceInfo)entry.getValue();
	    try
	    {
		info.service.stop();
	    }
	    catch(java.lang.Exception e)
	    {
		java.io.StringWriter sw = new java.io.StringWriter();
		java.io.PrintWriter pw = new java.io.PrintWriter(sw);
		e.printStackTrace(pw);
		pw.flush();
		_logger.warning("ServiceManager: exception in stop for service " + name + "\n" + sw.toString());
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
		    _logger.warning("ServiceManager: exception in stop for service " + name + "\n" + sw.toString());
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
		    _logger.warning("ServiceManager: exception in stop for service " + name + "\n" + sw.toString());
		}
	    }
	}

	_services.clear();
    }

    private void
    destroyCommunicator(String name, Ice.Communicator communicator)
    {
	if(communicator != null)
	{
	    try
	    {
		communicator.shutdown();
		communicator.waitForShutdown();
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
		_logger.warning("ServiceManager: exception in stop for service " + name + "\n" + sw.toString());
	    }
	}
	    
	if(communicator != null)
	{
	    try
	    {
		communicator.destroy();
	    }
	    catch(java.lang.Exception e)
	    {
		java.io.StringWriter sw = new java.io.StringWriter();
		java.io.PrintWriter pw = new java.io.PrintWriter(sw);
		e.printStackTrace(pw);
		pw.flush();
		_logger.warning("ServiceManager: exception in stop for service " + name + "\n" + sw.toString());
	    }
	}
    }

    class ServiceInfo
    {
        public Service service;
	public Ice.Communicator communicator = null;
    }

    private Ice.Application _server;
    private Ice.Logger _logger;
    private String[] _argv; // Filtered server argument vector
    private java.util.HashMap _services = new java.util.HashMap();
}
