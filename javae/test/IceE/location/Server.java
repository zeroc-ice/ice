// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

public class Server
{
    public static int
    run(String[] args, Ice.Communicator communicator, java.io.PrintStream out)
    {
	//
	// When running as a MIDlet the properties for the server may be
	// overridden by configuration. If it isn't then we assume
	// defaults.
	//
	if(communicator.getProperties().getProperty("ServerManagerAdapter.Endpoints").length() == 0)
	{
	    communicator.getProperties().setProperty("ServerManagerAdapter.Endpoints", "default -p 12345 -t 30000");
	}

	//
        // For blocking client test, set timeout so CloseConnection send will
        // return quickly. Otherwise server will hang since client is not 
        // listening for these messages.
        //
        if(communicator.getProperties().getProperty("Ice.Blocking") != "0")
        {
            communicator.getProperties().setProperty("Ice.Override.Timeout", "100");
        }

	//
	// Register the server manager. The server manager creates a new
	// 'server' (a server isn't a different process, it's just a new
	// communicator and object adapter).
	//
	Ice.ObjectAdapter adapter = communicator.createObjectAdapter("ServerManagerAdapter");

	//
	// We also register a sample server locator which implements the
	// locator interface, this locator is used by the clients and the
	// 'servers' created with the server manager interface.
	//
	ServerLocatorRegistry registry = new ServerLocatorRegistry();
	registry.addObject(adapter.createProxy(Ice.Util.stringToIdentity("ServerManager")));
	Ice.Object object = new ServerManagerI(adapter, registry);
	adapter.add(object, Ice.Util.stringToIdentity("ServerManager"));

	Ice.LocatorRegistryPrx registryPrx = 
	    Ice.LocatorRegistryPrxHelper.uncheckedCast(adapter.add(registry, Ice.Util.stringToIdentity("registry")));
	
	ServerLocator locator = new ServerLocator(registry, registryPrx);
	adapter.add(locator, Ice.Util.stringToIdentity("locator"));
	
	adapter.activate();
	communicator.waitForShutdown();
	
	return 0;
    }

    public static void
    main(String[] args)
    {
        int status = 0;
        Ice.Communicator communicator = null;

        try
        {
            communicator = Ice.Util.initialize(args);
            status = run(args, communicator, System.out);
        }
        catch(Ice.LocalException ex)
        {
            ex.printStackTrace();
            status = 1;
        }

        if(communicator != null)
        {
            try
            {
                communicator.destroy();
            }
            catch(Ice.LocalException ex)
            {
                ex.printStackTrace();
                status = 1;
            }
        }

	System.gc();
        System.exit(status);
    }
}
