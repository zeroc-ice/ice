// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public class Server : Ice.Application
{
    public override int run(string[] args)
    {
	args = communicator().getProperties().parseCommandLineOptions("TestAdapter", args);
        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("TestAdapter");
        adapter.add(new TestI(adapter), Ice.Util.stringToIdentity("test"));
	shutdownOnInterrupt();
	try
	{
	    adapter.activate();
	}
	catch(Ice.ObjectAdapterDeactivatedException ex)
	{
	}
        communicator().waitForShutdown();
        return 0;
    }

    public static void Main(string[] args)
    {
	Server server = new Server();
	int status = server.main(args);
	System.Environment.Exit(status);
    }
}
