// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceBox;

public final class Admin
{
    private static void usage()
    {
        System.err.println(
            "Usage: com.zeroc.IceBox.Admin [options] [command...]\n" +
            "Options:\n" +
            "-h, --help          Show this message.\n" +
            "\n" +
            "Commands:\n" +
            "start SERVICE       Start a service.\n" +
            "stop SERVICE        Stop a service.\n" +
            "shutdown            Shutdown the server.");
    }

    public static void main(String[] args)
    {
        int status = 0;
        java.util.List<String> commands = new java.util.ArrayList<String>();

        try(com.zeroc.Ice.Communicator communicator = com.zeroc.Ice.Util.initialize(args, commands))
        {
            Runtime.getRuntime().addShutdownHook(new Thread(() ->
            {
                communicator.destroy();
            }));

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
            else
            {
                status = run(communicator, commands);
            }
        }

        System.exit(status);
    }

    public static int run(com.zeroc.Ice.Communicator communicator, java.util.List<String> commands)
    {
        com.zeroc.Ice.ObjectPrx base = communicator.propertyToProxy("IceBoxAdmin.ServiceManager.Proxy");

        if(base == null)
        {
            //
            // The old deprecated way to retrieve the service manager proxy
            //

            com.zeroc.Ice.Properties properties = communicator.getProperties();

            com.zeroc.Ice.Identity managerIdentity = new com.zeroc.Ice.Identity();
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

                managerProxy = "\"" + communicator.identityToString(managerIdentity) + "\" :" +
                    managerEndpoints;
            }
            else
            {
                String managerAdapterId = properties.getProperty("IceBox.ServiceManager.AdapterId");
                if(managerAdapterId.length() == 0)
                {
                    System.err.println("IceBox.Admin: property `IceBoxAdmin.ServiceManager.Proxy' is not set");
                    return 1;
                }

                managerProxy = "\"" + communicator.identityToString(managerIdentity) + "\" @" +
                    managerAdapterId;
            }

            base = communicator.stringToProxy(managerProxy);
        }

        com.zeroc.IceBox.ServiceManagerPrx manager = com.zeroc.IceBox.ServiceManagerPrx.checkedCast(base);
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
                catch(com.zeroc.IceBox.NoSuchServiceException ex)
                {
                    System.err.println("IceBox.Admin: unknown service `" + service + "'");
                    return 1;
                }
                catch(com.zeroc.IceBox.AlreadyStartedException ex)
                {
                    System.err.println("IceBox.Admin: service already started.");
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
                catch(com.zeroc.IceBox.NoSuchServiceException ex)
                {
                    System.err.println("IceBox.Admin: unknown service `" + service + "'");
                    return 1;
                }
                catch(com.zeroc.IceBox.AlreadyStoppedException ex)
                {
                    System.err.println("IceBox.Admin: service already stopped.");
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
