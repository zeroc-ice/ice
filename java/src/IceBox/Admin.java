// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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

	    String managerIdentity = properties.getProperty("IceBox.ServiceManager.Identity");
	    if(managerIdentity.length() == 0)
	    {
		managerIdentity =
		    properties.getPropertyWithDefault("IceBox.InstanceName", "IceBox") + "/ServiceManager";
	    }

	    String managerProxy;

	    if(properties.getProperty("Ice.Default.Locator").length() == 0)
	    {
		String managerEndpoints = properties.getProperty("IceBox.ServiceManager.Endpoints");
		if(managerEndpoints.length() == 0)
		{
		    System.err.println(appName() + ": property `IceBox.ServiceManager.Endpoints' is not set");
		    return 1;
		}

		managerProxy = managerIdentity + ":" + managerEndpoints;
	    }
	    else
	    {
		String managerAdapterId = properties.getProperty("IceBox.ServiceManager.AdapterId");
		if(managerAdapterId.length() == 0)
		{
		    System.err.println(appName() + ": property `IceBox.ServiceManager.AdapterId' is not set");
		    return 1;
		}

		managerProxy = managerIdentity + ":" + managerAdapterId;
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
