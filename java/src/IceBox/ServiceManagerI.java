// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
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

        Ice.Properties properties = communicator.getProperties();
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
            // Create an object adapter. Services probably should NOT share
            // this object adapter, as the endpoint(s) for this object adapter
            // will most likely need to be firewalled for security reasons.
            //
            Ice.ObjectAdapter adapter =
                _communicator.createObjectAdapterFromProperty("ServiceManagerAdapter",
                                                              "IceBox.ServiceManager.Endpoints");
            adapter.add(this, Ice.Util.stringToIdentity("ServiceManager"));

            //
            // Load and initialize the services defined in the property set
            // with the prefix "IceBox.Service.". These properties should
            // have the following format:
            //
            // IceBox.Service.Foo=Package.Foo [args]
            //
            final String prefix = "IceBox.Service.";
            Ice.Properties properties = _communicator.getProperties();
            String[] services = properties.getProperties(prefix);
            for (int i = 0; i < services.length; i += 2)
            {
                String name = services[i].substring(prefix.length());
                String value = services[i + 1];

                //
                // Separate the entry point from the arguments.
                //
                String className;
                String[] args;
                int pos = value.indexOf(' ');
                if (pos == -1)
                {
                    pos = value.indexOf('\t');
                }
                if (pos == -1)
                {
                    pos = value.indexOf('\n');
                }
                if (pos == -1)
                {
                    className = value;
                    args = new String[0];
                }
                else
                {
                    className = value.substring(0, pos);
                    args = value.substring(pos).trim().split("[ \t\n]+", pos);
                }

                init(name, className, args);
            }

            //
            // Invoke start() on the services.
            //
            java.util.Iterator r = _services.entrySet().iterator();
            while (r.hasNext())
            {
                java.util.Map.Entry entry = (java.util.Map.Entry)r.next();
                String name = (String)entry.getKey();
                Service svc = (Service)entry.getValue();
                try
                {
                    svc.start();
                }
                catch (FailureException ex)
                {
                    throw ex;
                }
                catch (Exception ex)
                {
                    FailureException e = new FailureException();
                    e.reason = "ServiceManager: exception in start for service " + name + ": " + ex;
                    e.initCause(ex);
                    throw e;
                }
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
            if (bundleName.length() > 0)
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
        catch (FailureException ex)
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
        catch (Ice.LocalException ex)
        {
            java.io.StringWriter sw = new java.io.StringWriter();
            java.io.PrintWriter pw = new java.io.PrintWriter(sw);
            ex.printStackTrace(pw);
            pw.flush();
            _logger.error("ServiceManager: " + ex + "\n" + sw.toString());
            stopAll();
            return 1;
        }
        catch (Exception ex)
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

    private Service
    init(String service, String className, String[] args)
        throws FailureException
    {
        //
        // We need to create a property set to pass to init().
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
        for (int j = 0; j < _options.length; j++)
        {
            if (_options[j].startsWith("--" + service + "."))
            {
                l.add(_options[j]);
            }
        }
        for (int j = 0; j < args.length; j++)
        {
            l.add(args[j]);
        }
        for (int j = 0; j < _argv.length; j++)
        {
            if (_argv[j].startsWith("--" + service + "."))
            {
                l.add(_argv[j]);
            }
        }

        //
        // Create the service property set.
        //
        Ice.Util.addArgumentPrefix(service);
        Ice.StringSeqHolder argsH = new Ice.StringSeqHolder();
        argsH.value = new String[l.size()];
        l.toArray(argsH.value);
        Ice.Properties serviceProperties = Ice.Util.createProperties(argsH);

        //
        // Instantiate the class.
        //
        Service svc = null;
        try
        {
            Class c = Class.forName(className);
            java.lang.Object obj = c.newInstance();
            try
            {
                svc = (Service)obj;
            }
            catch (ClassCastException ex)
            {
                FailureException e = new FailureException();
                e.reason = "ServiceManager: class " + className + " does not implement IceBox.Service";
                throw e;
            }
        }
        catch (ClassNotFoundException ex)
        {
            FailureException e = new FailureException();
            e.reason = "ServiceManager: class " + className + " not found";
            e.initCause(ex);
            throw e;
        }
        catch (IllegalAccessException ex)
        {
            FailureException e = new FailureException();
            e.reason = "ServiceManager: unable to access default constructor in class " + className;
            e.initCause(ex);
            throw e;
        }
        catch (InstantiationException ex)
        {
            FailureException e = new FailureException();
            e.reason = "ServiceManager: unable to instantiate class " + className;
            e.initCause(ex);
            throw e;
        }

        //
        // Invoke Service::init().
        //
        try
        {
            svc.init(service, _communicator, serviceProperties, argsH.value);
            _services.put(service, svc);
        }
        catch (FailureException ex)
        {
            throw ex;
        }
        catch (Exception ex)
        {
            FailureException e = new FailureException();
            e.reason = "ServiceManager: exception while initializing service " + service + ": " + ex;
            e.initCause(ex);
            throw e;
        }

        return svc;
    }

    private void
    stop(String service)
        throws FailureException
    {
        Service svc = (Service)_services.remove(service);
        assert(svc != null);

        try
        {
            svc.stop();
        }
        catch (Exception ex)
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
        java.util.Iterator r = _services.keySet().iterator();
        while (r.hasNext())
        {
            String service = (String)r.next();
            try
            {
                stop(service);
            }
            catch (FailureException ex)
            {
                _logger.error(ex.reason);
            }
        }
        assert(_services.isEmpty());
    }

    private Ice.Communicator _communicator;
    private Ice.Logger _logger;
    private String[] _argv; // Filtered server argument vector
    private String[] _options; // Server property set converted to command-line options
    private java.util.HashMap _services = new java.util.HashMap();
}
