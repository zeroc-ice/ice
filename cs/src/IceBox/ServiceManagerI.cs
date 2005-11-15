// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections;

//
// NOTE: the class isn't final on purpose to allow users to eventually
// extend it.
//
class ServiceManagerI : IceBox.ServiceManagerDisp_
{
    public ServiceManagerI(string[] args)
    {
	_logger = Ice.Application.communicator().getLogger();
	_argv = args;
    }

    public override Ice.SliceChecksumDict
    getSliceChecksums(Ice.Current current)
    {
	return Ice.SliceChecksums.checksums;
    }

    public override void
    shutdown(Ice.Current current)
    {
	Ice.Application.communicator().shutdown();
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
	    Ice.ObjectAdapter adapter = Ice.Application.communicator().createObjectAdapter("IceBox.ServiceManager");

	    Ice.Properties properties = Ice.Application.communicator().getProperties();

	    string identity = properties.getProperty("IceBox.ServiceManager.Identity");
	    if(identity.Length == 0)
	    {
		identity = properties.getPropertyWithDefault("IceBox.InstanceName", "IceBox") + "/ServiceManager";
	    }
	    adapter.add(this, Ice.Util.stringToIdentity(identity));

	    //
	    // Parse the IceBox.LoadOrder property.
	    //
	    string order = properties.getProperty("IceBox.LoadOrder");
	    string[] loadOrder = null;
	    if(order.Length > 0)
	    {
		char[] seperators = { ',', '\t', ' ' };
		loadOrder = order.Trim().Split(seperators);
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
	    string prefix = "IceBox.Service.";
	    Ice.PropertyDict services = properties.getPropertiesForPrefix(prefix);
	    if(loadOrder != null)
	    {
		for(int i = 0; i < loadOrder.Length; ++i)
		{
		    if(loadOrder[i].Length > 0)
		    {
			string key = prefix + loadOrder[i];
			string value = services[key];
			if(value == null)
			{
			    IceBox.FailureException ex = new IceBox.FailureException();
			    ex.reason = "ServiceManager: no service definition for `" + loadOrder[i] + "'";
			    throw ex;
			}
			load(loadOrder[i], value);
			services.Remove(key);
		    }
		}
	    }

	    foreach(DictionaryEntry entry in services)
	    {
		string name = ((string)entry.Key).Substring(prefix.Length);
		string value = (string)entry.Value;
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
	    string bundleName = properties.getProperty("IceBox.PrintServicesReady");
	    if(bundleName.Length > 0)
	    {
		Console.Out.WriteLine(bundleName + " ready");
	    }

	    //
	    // Don't move after the adapter activation. This allows
	    // applications to wait for the service manager to be
	    // reachable before sending a signal to shutdown the
	    // IceBox.
	    //
	    Ice.Application.shutdownOnInterrupt();

	    //
	    // Start request dispatching after we've started the services.
	    //
	    try
	    {
		adapter.activate();
	    }
	    catch(Ice.ObjectAdapterDeactivatedException)
	    {
		//
		// Expected if the communicator has been shutdown.
		//
	    }

	    Ice.Application.communicator().waitForShutdown();
	    // XXX:
	    //Ice.Application.defaultInterrupt();

	    //
	    // Invoke stop() on the services.
	    //
	    stopAll();
	}
	catch(IceBox.FailureException ex)
	{
	    _logger.error(ex.ToString());
	    stopAll();
	    return 1;
	}
	catch(Ice.LocalException ex)
	{
	    _logger.error("ServiceManager: " + ex.ToString());
	    stopAll();
	    return 1;
	}
	catch(Exception ex)
	{
	    _logger.error("ServiceManager: unknown exception\n" + ex.ToString());
	    stopAll();
	    return 1;
	}

	return 0;
    }

    private void
    load(string name, string value)
    {
	//
	// Separate the entry point from the arguments.
	//
	string entryPoint = value;
	string[] args = new string[0];
	int start = value.IndexOf(':');
	if(start != -1)
	{
	    //
	    // Find the whitespace.
	    //
	    int pos = value.IndexOf(' ', start);
	    if(pos == -1)
	    {
		pos = value.IndexOf('\t', start);
	    }
	    if(pos == -1)
	    {
		pos = value.IndexOf('\n', start);
	    }
	    if(pos != -1)
	    {
		entryPoint = value.Substring(0, pos);
		char[] delims = { ' ', '\t', '\n' };
		args = value.Substring(pos).Trim().Split(delims, pos);
	    }
	}
	
	startService(name, entryPoint, args);
    }

    private void
    startService(string service, string entryPoint, string[] args)
    {
	//
	// Create the service property set from the service arguments
	// and the server arguments. The service property set will be
	// used to create a new communicator, or will be added to the
	// shared communicator, depending on the value of the
	// IceBox.UseSharedCommunicator property.
	//
	ArrayList l = new ArrayList();
	for(int j = 0; j < args.Length; j++)
	{
	    l.Add(args[j]);
	}
	for(int j = 0; j < _argv.Length; j++)
	{
	    if(_argv[j].StartsWith("--" + service + "."))
	    {
		l.Add(_argv[j]);
	    }
	}

	string[] serviceArgs = (string[])l.ToArray(typeof(string));

	//
	// Instantiate the class.
	//
	ServiceInfo info = new ServiceInfo();

	//
	// Retrieve the assembly name and the type.
	//
	string err = "ServiceManager: unable to load service '" + entryPoint + "': ";
	int sepPos = entryPoint.IndexOf(':');
	if (sepPos == -1)
	{
	    IceBox.FailureException e = new IceBox.FailureException();
	    e.reason = err + "invalid entry point format: " + entryPoint;
	    throw e;
	}
	
	System.Reflection.Assembly serviceAssembly = null;
	string assemblyName = entryPoint.Substring(0, sepPos);
	try
	{
	    if (System.IO.File.Exists(assemblyName))
	    {
		serviceAssembly = System.Reflection.Assembly.LoadFrom(assemblyName);
	    }
	    else
	    {
		serviceAssembly = System.Reflection.Assembly.Load(assemblyName);
	    }
	}
	catch(System.Exception ex)
	{
	    IceBox.FailureException e = new IceBox.FailureException(ex);
	    e.reason = err + "unable to load assembly: " + assemblyName;
	    throw e;
	}
	    
	//
	// Instantiate the class.
	//
	string className = entryPoint.Substring(sepPos + 1);
	System.Type c = serviceAssembly.GetType(className);
	if(c == null)
	{
	    IceBox.FailureException e = new IceBox.FailureException();
	    e.reason = err + "GetType failed for '" + className + "'";
	    throw e;
	}
	
	try
	{
	    info.service = (IceBox.Service)IceInternal.AssemblyUtil.createInstance(c);
	}
	catch(System.InvalidCastException ex)
	{
	    IceBox.FailureException e = new IceBox.FailureException(ex);
	    e.reason = err + "InvalidCastException to Ice.PluginFactory";
	    throw e;
	}
	catch(System.UnauthorizedAccessException ex)
	{
	    IceBox.FailureException e = new IceBox.FailureException(ex);
	    e.reason = err + "UnauthorizedAccessException";
	    throw e;
	}
	catch(System.Exception ex)
	{
	    IceBox.FailureException e = new IceBox.FailureException(ex);
	    e.reason = err + "System.Exception";
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
	    Ice.Properties properties = Ice.Application.communicator().getProperties();
	    if(properties.getPropertyAsInt("IceBox.UseSharedCommunicator." + service) > 0)
	    {
		Ice.Properties fileProperties = Ice.Util.createProperties(ref serviceArgs);
		properties.parseCommandLineOptions("", fileProperties.getCommandLineOptions());

		serviceArgs = properties.parseIceCommandLineOptions(serviceArgs);
		serviceArgs = properties.parseCommandLineOptions(service, serviceArgs);
	    }
	    else
	    {
		Ice.Properties serviceProperties = properties.ice_clone_();

		//
		// Initialize the Ice.ProgramName property with the name of this service.
		//
		string name = serviceProperties.getProperty("Ice.ProgramName");
		if(!name.Equals(service))
		{
		    name = name.Length == 0 ? service : name + "-" + service;
		}

		Ice.Properties fileProperties = Ice.Util.createProperties(ref serviceArgs);
		serviceProperties.parseCommandLineOptions("", fileProperties.getCommandLineOptions());
		serviceProperties.setProperty("Ice.ProgramName", name);

		serviceArgs = serviceProperties.parseIceCommandLineOptions(serviceArgs);
		serviceArgs = serviceProperties.parseCommandLineOptions(service, serviceArgs);

		string[] emptyArgs = new string[0];
		info.communicator = Ice.Util.initializeWithProperties(ref emptyArgs, serviceProperties);
	    }
	
	    Ice.Communicator communicator = info.communicator != null ? info.communicator :
		Ice.Application.communicator();

	    try
	    {
		info.service.start(service, communicator, serviceArgs);
	    }
	    catch(Exception)
	    {
		if(info.communicator != null)
		{
		    try
		    {
			info.communicator.shutdown();
			info.communicator.waitForShutdown();
		    }
		    catch(Ice.CommunicatorDestroyedException)
		    {
			//
			// Ignore, the service might have already destroyed
			// the communicator for its own reasons.
			//
		    }
		    catch(Exception e)
		    {
			_logger.warning("ServiceManager: exception in shutting down communicator for service "
					+ service + "\n" + e.ToString());
		    }
		    
		    try
		    {
			info.communicator.destroy();
		    }
		    catch(Exception e)
		    {
			_logger.warning("ServiceManager: exception in destroying communciator for service"
					+ service + "\n" + e.ToString());
		    }
		}
		throw;
	    }

	    _services[service] = info;
	}
	catch(IceBox.FailureException)
	{
	    throw;
	}
	catch(Exception ex)
	{
	    IceBox.FailureException e = new IceBox.FailureException(ex);
	    e.reason = "ServiceManager: exception while starting service " + service + ": " + ex;
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
	foreach(DictionaryEntry entry in _services)
	{
	    string name = (string)entry.Key;
	    ServiceInfo info = (ServiceInfo)entry.Value;
	    try
	    {
		info.service.stop();
	    }
	    catch(Exception e)
	    {
		_logger.warning("ServiceManager: exception in stop for service " + name + "\n" + e.ToString());
	    }

	    if(info.communicator != null)
	    {
		try
		{
		    info.communicator.shutdown();
		    info.communicator.waitForShutdown();
		}
		catch(Ice.CommunicatorDestroyedException)
		{
		    //
		    // Ignore, the service might have already destroyed
		    // the communicator for its own reasons.
		    //
		}
		catch(Exception e)
		{
		    _logger.warning("ServiceManager: exception in stop for service " + name + "\n" + e.ToString());
		}
	    
		try
		{
		    info.communicator.destroy();
		}
		catch(Exception e)
		{
		    _logger.warning("ServiceManager: exception in stop for service " + name + "\n" + e.ToString());
		}
	    }
	}

	_services.Clear();
    }

    struct ServiceInfo
    {
        public IceBox.Service service;
	public Ice.Communicator communicator;
    }

    private Ice.Logger _logger;
    private string[] _argv; // Filtered server argument vector
    private Hashtable _services = new Hashtable(); // String/ServiceInfo pairs.
}
