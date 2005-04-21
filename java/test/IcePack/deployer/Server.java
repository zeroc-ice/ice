// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public class Server extends Ice.Application
{
    public int
    run(String[] args)
    {
	Ice.Properties properties = communicator().getProperties();
	String name = properties.getProperty("Ice.ProgramName");

	Ice.StringSeqHolder argsH = new Ice.StringSeqHolder(args);
	argsH.value = properties.parseCommandLineOptions("Test", argsH.value);

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
	defaultInterrupt();
        return 0;
    }

    public static void
    main(String[] args)
    {
	Server server = new Server();
	int status = server.main("test.IcePack.deploy.Server", args);
	System.gc();
	System.exit(status);
    }
}
