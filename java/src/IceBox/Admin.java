// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
                "shutdown            Shutdown the server.");
        }

        public int
        run(String[] args)
        {
            Ice.Properties properties = communicator().getProperties();

            args = properties.parseCommandLineOptions("IceBox", args);

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

            final String managerEndpointsProperty = "IceBox.ServiceManager.Endpoints";
            String managerEndpoints = properties.getProperty(managerEndpointsProperty);
            if(managerEndpoints.length() == 0)
            {
                System.err.println(appName() + ": property `" + managerEndpointsProperty + "' is not set");
                return 1;
            }

            Ice.ObjectPrx base = communicator().stringToProxy("ServiceManager:" + managerEndpoints);
            IceBox.ServiceManagerPrx manager = IceBox.ServiceManagerPrxHelper.checkedCast(base);
            if(manager == null)
            {
                System.err.println(appName() + ": `" + managerEndpoints + "' is not running");
                return 1;
            }

            for(int i = 0; i < commands.size(); i++)
            {
                String command = (String)commands.get(i);
                if(command.equals("shutdown"))
                {
                    manager.shutdown();
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
