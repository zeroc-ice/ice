// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

public class Server
{
    private static int
    run(String[] args, Ice.Communicator communicator)
    {
	//
	// Register the server manager. The server manager creates a new
	// 'server' (a server isn't a different process, it's just a new
	// communicator and object adapter).
	//
	Ice.ObjectAdapter adapter = communicator.createObjectAdapterWithEndpoints("ServerManagerAdapter", 
										  "default -p 12345");

	Ice.Object object = new ServerManagerI(adapter);
	adapter.add(object, Ice.Util.stringToIdentity("servermanager"));

	//
	// We also register a sample server locator which implements the
	// locator interface, this locator is used by the clients and the
	// 'servers' created with the server manager interface.
	//
	ServerLocatorRegistry registry = new ServerLocatorRegistry();
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
            status = run(args, communicator);
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

        System.exit(status);
    }
}
