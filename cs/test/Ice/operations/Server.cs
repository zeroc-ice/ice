// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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
	communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -p 12345 -t 2000");
	Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
	Ice.Identity id = Ice.Util.stringToIdentity("test");
	adapter.add(new MyDerivedClassI(adapter, id), id);
	adapter.add(new TestCheckedCastI(), Ice.Util.stringToIdentity("context"));
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
	
	Environment.Exit(status);
    }
}
