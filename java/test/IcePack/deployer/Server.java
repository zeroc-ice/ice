// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

public class Server extends Ice.Application
{
    public int
    run(String[] args)
    {
	Ice.Properties properties = communicator().getProperties();
	String name = properties.getProperty("Ice.ProgramName");

        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("Server");
        Ice.Object object = new TestI(adapter, properties);
        adapter.add(object, Ice.Util.stringToIdentity(name));
	shutdownOnInterrupt();
	try
	{
	    adapter.activate();
	}
	catch(Ice.ObjectAdapterDeactivatedException ex)
	{
	}
        communicator().waitForShutdown();
	ignoreInterrupt();
        return 0;
    }

    public static void
    main(String[] args)
    {
	Server server = new Server();
	server.main("test.IcePack.deploy.Server", args);
    }
}
