// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceBox;

public final class Admin
{
    static class ShutdownHook implements Runnable
    {
        private Ice.Communicator communicator;

        ShutdownHook(Ice.Communicator communicator)
        {
            this.communicator = communicator;
        }

        @Override
        public void
        run()
        {
            communicator.destroy();
        }
    }

    private static void
    usage()
    {
        System.err.println(
            "Usage: IceBox.Admin [options] [command...]\n" +
            "Options:\n" +
            "-h, --help          Show this message.\n" +
            "\n" +
            "Commands:\n" +
            "start SERVICE       Start a service.\n" +
            "stop SERVICE        Stop a service.\n" +
            "shutdown            Shutdown the server.");
    }

    public static void
    main(String[] args)
    {
        int status = 0;
        Ice.StringSeqHolder argHolder = new Ice.StringSeqHolder(args);

        try(Ice.Communicator communicator = Ice.Util.initialize(argHolder))
        {
            Runtime.getRuntime().addShutdownHook(new Thread(new ShutdownHook(communicator)));

            java.util.List<String> commands = new java.util.ArrayList<String>(java.util.Arrays.asList(argHolder.value));

            for(String command : commands)
            {
                if(command.equals("-h") || command.equals("--help"))
                {
                    usage();
                    status = 1;
                    break;
                }
                else if(command.charAt(0) == '-')
                {
                    System.err.println("IceBox.Admin: unknown option `" + command + "'");
                    usage();
                    status = 1;
                    break;
                }
            }

            if(commands.isEmpty())
            {
                usage();
                status = 0;
            }
            else if(status == 0)
            {
                status = run(communicator, commands);
            }
        }

        System.exit(status);
    }

    public static int
    run(Ice.Communicator communicator, java.util.List<String> commands)
    {
        Ice.ObjectPrx base = communicator.propertyToProxy("IceBoxAdmin.ServiceManager.Proxy");

        if(base == null)
        {
            //
            // The old deprecated way to retrieve the service manager proxy
            //

            Ice.Properties properties = communicator.getProperties();

            Ice.Identity managerIdentity = new Ice.Identity();
            managerIdentity.category = properties.getPropertyWithDefault("IceBox.InstanceName", "IceBox");
            managerIdentity.name = "ServiceManager";

            String managerProxy;
            if(properties.getProperty("Ice.Default.Locator").length() == 0)
            {
                String managerEndpoints = properties.getProperty("IceBox.ServiceManager.Endpoints");
                if(managerEndpoints.length() == 0)
                {
                    System.err.println("IceBox.Admin: property `IceBoxAdmin.ServiceManager.Proxy' is not set");
                    return 1;
                }

                managerProxy = "\"" + communicator.identityToString(managerIdentity) + "\" :" + managerEndpoints;
            }
            else
            {
                String managerAdapterId = properties.getProperty("IceBox.ServiceManager.AdapterId");
                if(managerAdapterId.length() == 0)
                {
                    System.err.println("IceBox.Admin: property `IceBoxAdmin.ServiceManager.Proxy' is not set");
                    return 1;
                }

                managerProxy = "\"" + communicator.identityToString(managerIdentity) + "\" @" + managerAdapterId;
            }

            base = communicator.stringToProxy(managerProxy);
        }

        IceBox.ServiceManagerPrx manager = IceBox.ServiceManagerPrxHelper.checkedCast(base);
        if(manager == null)
        {
            System.err.println("IceBox.Admin: `" + base.toString() + "' is not an IceBox::ServiceManager");
            return 1;
        }

        for(int i = 0; i < commands.size(); i++)
        {
            String command = commands.get(i);
            if(command.equals("shutdown"))
            {
                manager.shutdown();
            }
            else if(command.equals("start"))
            {
                if(++i >= commands.size())
                {
                    System.err.println("IceBox.Admin: no service name specified.");
                    return 1;
                }

                String service = commands.get(i);
                try
                {
                    manager.startService(service);
                }
                catch(IceBox.NoSuchServiceException ex)
                {
                    System.err.println("IceBox.Admin: unknown service `" + service + "'");
                    return 1;
                }
                catch(IceBox.AlreadyStartedException ex)
                {
                    System.err.println("IceBox.Admin service already started.");
                }
            }
            else if(command.equals("stop"))
            {
                if(++i >= commands.size())
                {
                    System.err.println("IceBox.Admin: no service name specified.");
                    return 1;
                }

                String service = commands.get(i);
                try
                {
                    manager.stopService(service);
                }
                catch(IceBox.NoSuchServiceException ex)
                {
                    System.err.println("IceBox.Admin: unknown service `" + service + "'");
                    return 1;
                }
                catch(IceBox.AlreadyStoppedException ex)
                {
                    System.err.println("IceBox.Admin service already stopped.");
                }
            }
            else
            {
                System.err.println("IceBox.Admin: unknown command `" + command + "'");
                usage();
                return 1;
            }
        }

        return 0;
    }
}
