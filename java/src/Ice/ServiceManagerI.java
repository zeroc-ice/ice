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
        try
        {
            //
            // Initialize a Communicator. The services may share this
            // Communicator instance if desired.
            //
            _communicator = Util.initialize(args);

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
            initServices();

            //
            // Don't dispatch requests until we've initialized the services.
            //
            adapter.activate();

            //
            // Invoke start() on the services.
            //
            startServices();

            _communicator.waitForShutdown();

            //
            // Invoke stop() on the services.
            //
            stopServices();
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
    initServices()
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
            // Instantiate the class and invoke init().
            //
            try
            {
                Class c = Class.forName(className);
                java.lang.Object obj = c.newInstance();
                try
                {
                    Service svc = (Service)obj;
                    svc.init(_communicator, args);
                    _services.add(svc);
                }
                catch (ClassCastException ex)
                {
                    System.err.println("ServiceManager: class " + className + " does not implement Ice.Service");
                    return false;
                }
            }
            catch (ClassNotFoundException ex)
            {
                System.err.println("ServiceManager: class " + className + " not found");
                return false;
            }
            catch (IllegalAccessException ex)
            {
                System.err.println("ServiceManager: unable to access default constructor in class " + className);
                return false;
            }
            catch (InstantiationException ex)
            {
                System.err.println("ServiceManager: unable to instantiate class " + className);
                return false;
            }
        }

        return true;
    }

    private void
    startServices()
    {
        java.util.Iterator p = _services.iterator();
        while (p.hasNext())
        {
            Service svc = (Service)p.next();
            svc.start();
        }
    }

    private void
    stopServices()
    {
        java.util.Iterator p = _services.iterator();
        while (p.hasNext())
        {
            Service svc = (Service)p.next();
            svc.stop();
        }
    }

    public static void
    main(String[] args)
    {
        ServiceManagerI impl = new ServiceManagerI();
        int status = impl.run(args);
        System.exit(status);
    }

    private Communicator _communicator;
    private java.util.LinkedList _services = new java.util.LinkedList();
}
