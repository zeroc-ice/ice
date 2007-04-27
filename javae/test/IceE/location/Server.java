// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

public class Server
{
    public static int
    run(String[] args, Ice.Communicator communicator, Ice.InitializationData initData, java.io.PrintStream out)
    {
	//
	// When running as a MIDlet the properties for the server may be
	// overridden by configuration. If it isn't then we assume
	// defaults.
	//
	if(communicator.getProperties().getProperty("ServerManagerAdapter.Endpoints").length() == 0)
	{
	    communicator.getProperties().setProperty("ServerManagerAdapter.Endpoints", "default -p 12010 -t 30000");
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
	registry.addObject(adapter.createProxy(communicator.stringToIdentity("ServerManager")));
	Ice.Object object = new ServerManagerI(adapter, registry, initData);
	adapter.add(object, communicator.stringToIdentity("ServerManager"));

	Ice.LocatorRegistryPrx registryPrx = 
	    Ice.LocatorRegistryPrxHelper.uncheckedCast(adapter.add(registry, communicator.stringToIdentity("registry")));
	
	ServerLocator locator = new ServerLocator(registry, registryPrx);
	adapter.add(locator, communicator.stringToIdentity("locator"));
	
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
	    Ice.StringSeqHolder argsH = new Ice.StringSeqHolder(args);
	    Ice.InitializationData initData = new Ice.InitializationData();
	    initData.properties = Ice.Util.createProperties(argsH);

	    //
            // For blocking client test, set timeout so CloseConnection send will
            // return quickly. Otherwise server will hang since client is not 
            // listening for these messages.
            //
	    if(initData.properties.getPropertyAsInt("Ice.Blocking") > 0)
            {
                initData.properties.setProperty("Ice.Override.Timeout", "100"); 
	        initData.properties.setProperty("Ice.Warn.Connections", "0");
	    }

            communicator = Ice.Util.initialize(argsH, initData);
            status = run(argsH.value, communicator, initData, System.out);
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
