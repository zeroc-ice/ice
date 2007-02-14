// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceBox;

public final class Admin
{
    private static class Client extends Ice.Application
    {
        private void
        usage()
        {
            System.err.println(
                "Usage: " + appName() + " [options] [command...]\n" +
                "Options:\n" +
                "-h, --help          Show this message.\n" +
                "\n" +
                "Commands:\n" +
                "start SERVICE       Start a service." +
                "stop SERVICE        Stop a service." +
                "shutdown            Shutdown the server.");
        }

        public int
        run(String[] args)
        {
            java.util.ArrayList commands = new java.util.ArrayList();

            int idx = 0;
            while(idx < args.length)
            {
                if(args[idx].equals("-h") || args[idx].equals("--help"))
                {
                    usage();
                    return 1;
                }
                else if(args[idx].charAt(0) == '-')
                {
                    System.err.println(appName() + ": unknown option `" + args[idx] + "'");
                    usage();
                    return 1;
                }
                else
                {
                    commands.add(args[idx]);
                    ++idx;
                }
            }

            if(commands.isEmpty())
            {
                usage();
                return 0;
            }

            Ice.Properties properties = communicator().getProperties();

            Ice.Identity managerIdentity = new Ice.Identity();
            managerIdentity.category = properties.getPropertyWithDefault("IceBox.InstanceName", "IceBox");
            managerIdentity.name = "ServiceManager";

            String managerProxy;
            if(properties.getProperty("Ice.Default.Locator").length() == 0)
            {
                String managerEndpoints = properties.getProperty("IceBox.ServiceManager.Endpoints");
                if(managerEndpoints.length() == 0)
                {
                    System.err.println(appName() + ": property `IceBox.ServiceManager.Endpoints' is not set");
                    return 1;
                }

                managerProxy = "\"" + communicator().identityToString(managerIdentity) + "\" :" + managerEndpoints;
            }
            else
            {
                String managerAdapterId = properties.getProperty("IceBox.ServiceManager.AdapterId");
                if(managerAdapterId.length() == 0)
                {
                    System.err.println(appName() + ": property `IceBox.ServiceManager.AdapterId' is not set");
                    return 1;
                }

                managerProxy = "\"" + communicator().identityToString(managerIdentity) + "\" @" + managerAdapterId;
            }

            Ice.ObjectPrx base = communicator().stringToProxy(managerProxy);
            IceBox.ServiceManagerPrx manager = IceBox.ServiceManagerPrxHelper.checkedCast(base);
            if(manager == null)
            {
                System.err.println(appName() + ": `" + managerProxy + "' is not running");
                return 1;
            }

            for(int i = 0; i < commands.size(); i++)
            {
                String command = (String)commands.get(i);
                if(command.equals("shutdown"))
                {
                    manager.shutdown();
                }
                else if(command.equals("start"))
                {
                    if(++i >= commands.size())
                    {
                        System.err.println(appName() + ": no service name specified.");
                        return 1;
                    }

                    String service = (String)commands.get(i);
                    try
                    {
                        manager.startService(service);
                    }
                    catch(IceBox.NoSuchServiceException ex)
                    {
                        System.err.println(appName() + ": unknown service `" + service + "'");
                    }
                    catch(IceBox.AlreadyStartedException ex)
                    {
                        System.err.println(appName() + "service already started.");
                    }
                }
                else if(command.equals("stop"))
                {
                    if(++i >= commands.size())
                    {
                        System.err.println(appName() + ": no service name specified.");
                        return 1;
                    }

                    String service = (String)commands.get(i);
                    try
                    {
                        manager.stopService(service);
                    }
                    catch(IceBox.NoSuchServiceException ex)
                    {
                        System.err.println(appName() + ": unknown service `" + service + "'");
                    }
                    catch(IceBox.AlreadyStoppedException ex)
                    {
                        System.err.println(appName() + "service already stopped.");
                    }
                }
                else
                {
                    System.err.println(appName() + ": unknown command `" + command + "'");
                    usage();
                    return 1;
                }
            }

            return 0;
        }
    }

    public static void
    main(String[] args)
    {
        Client app = new Client();
        int rc = app.main("IceBox.Admin", args);

        System.exit(rc);
    }
}
