// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
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

        Ice.ObjectAdapter adapter = communicator().createObjectAdapter(name);
        Ice.Object object = new TestI(adapter, properties);
        adapter.add(object, Ice.Util.stringToIdentity(name));
        adapter.activate();
	shutdownOnInterrupt();
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
