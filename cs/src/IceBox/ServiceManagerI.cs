// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
    startService(string name, Ice.Current current)
    {
        lock(this)
        {
            //
            // Search would be more efficient if services were contained in
            // a map, but order is required for shutdown.
            //
            for(int i = 0; i < _services.Count; ++i)
            {
                ServiceInfo info = (ServiceInfo)_services[i];
                if(info.name.Equals(name))
                {
                    if(info.active)
                    {
                        throw new IceBox.AlreadyStartedException();
                    }

                    try
                    {
                        info.service.start(info.name, info.communicator == null ? Ice.Application.communicator() 
                                                                                : info.communicator, info.args);
                        info.active = true;
                        _services[i] = info;
                    }
                    catch(Exception e)
                    {
                        _logger.warning("ServiceManager: exception in start for service " + info.name + "\n" + 
                                        e.ToString());
                    }

                    return;
                }
            }

            throw new IceBox.NoSuchServiceException();
        }
    }

    public override void
    stopService(string name, Ice.Current current)
    {
        lock(this)
        {
            //
            // Search would be more efficient if services were contained in
            // a map, but order is required for shutdown.
            //
            for(int i = 0; i < _services.Count; ++i)
            {
                ServiceInfo info = (ServiceInfo)_services[i];
                if(info.name.Equals(name))
                {
                    if(!info.active)
                    {
                        throw new IceBox.AlreadyStoppedException();
                    }

                    try
                    {
                        info.service.stop();
                        info.active = false;
                        _services[i] = info;
                    }
                    catch(Exception e)
                    {
                        _logger.warning("ServiceManager: exception in stop for service " + info.name + "\n" + 
                                        e.ToString());
                    }

                    return;
                }
            }

            throw new IceBox.NoSuchServiceException();
        }
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

            Ice.Identity identity = new Ice.Identity();
            identity.category = properties.getPropertyWithDefault("IceBox.InstanceName", "IceBox");
            identity.name = "ServiceManager";
            adapter.add(this, identity);

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
                try
                {
                    args = IceUtil.Options.split(value.Substring(pos));
                }
                catch(IceUtil.Options.BadQuote ex)
                {
                    IceBox.FailureException e = new IceBox.FailureException();
                    e.reason = "ServiceManager: invalid arguments for service `" + name + "':\n" + ex.ToString();
                    throw e;
                }
            }
        }
        
        startService(name, entryPoint, args);
    }

    private void
    startService(string service, string entryPoint, string[] args)
    {
        lock(this)
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

            //
            // Instantiate the class.
            //
            ServiceInfo info = new ServiceInfo();
            info.name = service;
            info.args = (string[])l.ToArray(typeof(string));

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
                if(info.service == null)
                {
                    IceBox.FailureException e = new IceBox.FailureException();
                    e.reason = err + "Can't find constructor for '" + className + "'";
                    throw e;
                }
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
                    Ice.Properties serviceProperties = Ice.Util.createProperties(ref info.args, properties);

                    //
                    // Erase properties in 'properties'
                    //
                    Ice.PropertyDict allProps = properties.getPropertiesForPrefix("");
                    foreach(string key in allProps.Keys)
                    {
                        if(serviceProperties.getProperty(key).Length == 0)
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
                    info.args = properties.parseCommandLineOptions(service, info.args);
                }
                else
                {
                    string name = properties.getProperty("Ice.ProgramName");
                    Ice.Properties serviceProperties;
                    if(properties.getPropertyAsInt("IceBox.InheritProperties") > 0)
                    {
                        serviceProperties = Ice.Util.createProperties(ref info.args, properties);
                    }
                    else
                    {
                        serviceProperties = Ice.Util.createProperties(ref info.args);
                    }

                    if(name.Equals(serviceProperties.getProperty("Ice.ProgramName")))
                    {
                        //
                        // If the service did not set its own program-name, and 
                        // the icebox program-name != service, append the service name to the 
                        // program name.
                        //
                        if(!name.Equals(service))
                        {
                            name = name.Length == 0 ? service : name + "-" + service;
                        }
                        serviceProperties.setProperty("Ice.ProgramName", name);
                    }
                
                    //
                    // Parse <service>.* command line options.
                    // (the Ice command line options were parsed by the createProperties above)
                    //
                    info.args = serviceProperties.parseCommandLineOptions(service, info.args);

                    Ice.InitializationData initData = new Ice.InitializationData();
                    initData.properties = serviceProperties;
                    info.communicator = Ice.Util.initialize(ref info.args, initData);
                }
        
                Ice.Communicator communicator = info.communicator != null ? info.communicator :
                    Ice.Application.communicator();

                try
                {
                    info.service.start(service, communicator, info.args);
                    info.active = true;
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

                _services.Add(info);
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
    }

    private void
    stopAll()
    {
        lock(this)
        {
            //
            // First, for each service, we call stop on the service and flush its database environment to 
            // the disk. Services are stopped in the reverse order of which they were started.
            //
            _services.Reverse();
            foreach(ServiceInfo info in _services)
            {
                if(info.active)
                {
                    try
                    {
                        info.service.stop();
                    }
                    catch(Exception e)
                    {
                        _logger.warning("ServiceManager: exception in stop for service " + info.name + "\n" +
                                        e.ToString());
                    }
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
                        _logger.warning("ServiceManager: exception in stop for service " + info.name + "\n" +
                                        e.ToString());
                    }
            
                    try
                    {
                        info.communicator.destroy();
                    }
                    catch(Exception e)
                    {
                        _logger.warning("ServiceManager: exception in stop for service " + info.name + "\n" +
                                        e.ToString());
                    }
                }
            }

            _services.Clear();
        }
    }

    struct ServiceInfo
    {
        public string name;
        public IceBox.Service service;
        public Ice.Communicator communicator;
        public bool active;
        public string[] args;
    }

    private Ice.Logger _logger;
    private string[] _argv; // Filtered server argument vector
    private ArrayList _services = new ArrayList(); // ServiceInfo
}
