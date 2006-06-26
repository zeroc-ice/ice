// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;

public class Server
{
    private static int run(string[] args, Ice.Communicator communicator)
    {
	//
	// We don't want connection warnings because of the timeout test.
	//
	communicator.getProperties().setProperty("Ice.Warn.Connections", "0");
	
	communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -p 12010 -t 2000:udp");
	Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
	Ice.Identity id = communicator.stringToIdentity("test");
	adapter.add(new MyDerivedClassI(adapter, id), id);
	adapter.add(new TestCheckedCastI(), communicator.stringToIdentity("context"));
	adapter.activate();

	communicator.waitForShutdown();
	return 0;
    }
    
    public static void Main(string[] args)
    {
	int status = 0;
	Ice.Communicator communicator = null;
	
	try
	{
	    communicator = Ice.Util.initialize(ref args);
	    status = run(args, communicator);
	}
	catch(Ice.LocalException ex)
	{
	    Console.Error.WriteLine(ex);
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
		Console.Error.WriteLine(ex);
		status = 1;
	    }
	}
	
        if(status != 0)
        {
            System.Environment.Exit(status);
        }
    }
}
