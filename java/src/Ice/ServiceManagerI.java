// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package Ice;

public final class ServiceManagerI extends _ServiceManagerDisp
{
    public
    ServiceManagerI()
    {
    }

    public void
    shutdown(Ice.Current current)
    {
        _communicator.shutdown();
    }

    private int
    run(String[] args)
    {
        StringSeqHolder argsH = new StringSeqHolder(args);
        try
        {
            //
            // Initialize a Communicator. The services may share this
            // Communicator instance if desired.
            //
            Util.addArgumentPrefix("IceBox");
            StringSeqHolder argsH = new StringSeqHolder(args);
            _communicator = Util.initialize(argsH);
        }
        catch(Exception ex)
        {
            // Don't have a logger yet
            System.err.println("ServiceManager: exception in Communicator");
            ex.printStackTrace();
            return 1;
        }

        try
        {
            _logger = _communicator.getLogger();

            //
            // Create an object adapter. Services probably should NOT share
            // this object adapter, as the endpoint(s) for this object adapter
            // will most likely need to be firewalled for security reasons.
            //
            Ice.ObjectAdapter adapter = _communicator.createObjectAdapter("ServiceManagerAdapter");
            adapter.add(this, Ice.Util.stringToIdentity("ServiceManager"));

            //
            // Load and initialize the services.
            //
            if (!initServices(argsH.value))
            {
                stopServices();
                return 1;
            }

            //
            // Invoke start() on the services.
            //
            if (!startServices())
            {
                stopServices();
                return 1;
            }

            //
            // We may want to notify external scripts that the services have started.
            // This is done by defining IceBox.PrintServicesReady=bundleName
            // Where bundleName is whatever you choose to call this set of services.
            // It will be echoed back as "bundleName ready".
            //
            String bundleName = _communicator.getProperties().getProperty("IceBox.PrintServicesReady");
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
            stopServices();
        }
        catch(LocalException ex)
        {
            java.io.StringWriter sw = new java.io.StringWriter();
            java.io.PrintWriter pw = new java.io.PrintWriter(sw);
            ex.printStackTrace(pw);
            pw.flush();
            _logger.error("ServiceManager: " + ex + "\n" + sw.toString());
            stopServices();
            return 1;
        }
        catch(Exception ex)
        {
            java.io.StringWriter sw = new java.io.StringWriter();
            java.io.PrintWriter pw = new java.io.PrintWriter(sw);
            ex.printStackTrace(pw);
            pw.flush();
            _logger.error("ServiceManager: unknown exception\n" + sw.toString());
            stopServices();
            return 1;
        }

        if (_communicator != null)
        {
            try
            {
                _communicator.destroy();
            }
            catch(LocalException ex)
            {
                System.err.println("ServiceManager: " + ex);
                ex.printStackTrace();
                return 1;
            }
            catch(Exception ex)
            {
                System.err.println("ServiceManager: unknown exception");
                ex.printStackTrace();
                return 1;
            }
        }

        return 0;
    }

    private boolean
    initServices(String[] serverArgs)
    {
        //
        // Retrieve all properties with the prefix "Ice.Service.".
        // These properties should have the following format:
        //
        // Ice.Service.Foo=Package.Foo [args]
        //
        final String prefix = "Ice.Service.";
        Properties properties = _communicator.getProperties();
        String[] arr = properties.getProperties(prefix);
        for (int i = 0; i < arr.length; i += 2)
        {
            String name = arr[i].substring(prefix.length());
            String value = arr[i + 1];
            int pos = value.indexOf(' ');
            if (pos == -1)
            {
                pos = value.indexOf('\t');
            }
            if (pos == -1)
            {
                pos = value.indexOf('\n');
            }

            //
            // Separate the class name from the arguments.
            //
            String className;
            String[] args;
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

            //
            // Create a Property set from the server's remaining command-line
            // arguments, and the service's command-line arguments.
            //
            Util.addArgumentPrefix(name);
            StringSeqHolder argsH = new StringSeqHolder();
            argsH.value = new String[serverArgs.length + args.length];
            System.arraycopy(serverArgs, 0, argsH.value, 0, serverArgs.length);
            System.arraycopy(args, 0, argsH.value, serverArgs.length, args.length);
            Properties serviceProperties = Util.createProperties(argsH);

            //
            // TODO: Need removeArgumentPrefix? Without it, args for other
            // services which have already been initialized will also be
            // included in the property set.
            //

            //
            // TODO: Should we really supply the unfiltered *server*
            // command-line arguments to the service? If not, we may need
            // to modify the Properties interface to allow parsing
            // multiple sets of arguments.
            //

            //
            // Instantiate the class and invoke init().
            //
            try
            {
                Class c = Class.forName(className);
                java.lang.Object obj = c.newInstance();
                Service svc;
                try
                {
                    svc = (Service)obj;
                }
                catch (ClassCastException ex)
                {
                    _logger.error("ServiceManager: class " + className + " does not implement Ice.Service");
                    return false;
                }
                svc.init(name, _communicator, serviceProperties, argsH.value);
                _services.put(name, svc);
            }
            catch (ClassNotFoundException ex)
            {
                _logger.error("ServiceManager: class " + className + " not found");
                return false;
            }
            catch (IllegalAccessException ex)
            {
                _logger.error("ServiceManager: unable to access default constructor in class " + className);
                return false;
            }
            catch (InstantiationException ex)
            {
                _logger.error("ServiceManager: unable to instantiate class " + className);
                return false;
            }
            catch (ServiceFailureException ex)
            {
                java.io.StringWriter sw = new java.io.StringWriter();
                java.io.PrintWriter pw = new java.io.PrintWriter(sw);
                ex.printStackTrace(pw);
                pw.flush();
                _logger.error("ServiceManager: initialization failed for service " + name + "\n" + sw.toString());
                return false;
            }
        }

        return true;
    }

    private boolean
    startServices()
    {
        java.util.Iterator p = _services.entrySet().iterator();
        while (p.hasNext())
        {
            java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
            String name = (String)entry.getKey();
            Service svc = (Service)entry.getValue();
            try
            {
                svc.start();
            }
            catch (ServiceFailureException ex)
            {
                java.io.StringWriter sw = new java.io.StringWriter();
                java.io.PrintWriter pw = new java.io.PrintWriter(sw);
                ex.printStackTrace(pw);
                pw.flush();
                _logger.error("ServiceManager: start failed for service " + name + "\n" + sw.toString());
                return false;
            }
            catch(Exception ex)
            {
                java.io.StringWriter sw = new java.io.StringWriter();
                java.io.PrintWriter pw = new java.io.PrintWriter(sw);
                ex.printStackTrace(pw);
                pw.flush();
                _logger.error("ServiceManager: exception in start for service " + name + "\n" + sw.toString());
                return false;
            }
        }

        return true;
    }

    private void
    stopServices()
    {
        java.util.Iterator p = _services.entrySet().iterator();
        while (p.hasNext())
        {
            java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
            String name = (String)entry.getKey();
            Service svc = (Service)entry.getValue();
            try
            {
                svc.stop();
            }
            catch (Exception ex)
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

    public static void
    main(String[] args)
    {
        ServiceManagerI impl = new ServiceManagerI();
        int status = impl.run(args);
        System.exit(status);
    }

    private Communicator _communicator;
    private Logger _logger;
    private java.util.HashMap _services = new java.util.HashMap();
}
