// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;

public class Collocated
{
    private static int run(String[] args, Ice.Communicator communicator)
    {
        communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -p 12345 -t 2000");
        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
        Ice.Object obj = new MyDerivedClassI(adapter, Ice.Util.stringToIdentity("test"));
        adapter.add(obj, Ice.Util.stringToIdentity("test"));

	//
	// Make a separate adapter with a servant locator. We use this to test
	// that ::Ice::Context is correctly passed to checkedCast() operation.
	//
	communicator.getProperties().setProperty("CheckedCastAdapter.Endpoints", "default -p 12346 -t 10000");
	adapter = communicator.createObjectAdapter("CheckedCastAdapter");
	Ice.ServantLocator checkedCastLocator = new CheckedCastLocator();
	adapter.addServantLocator(checkedCastLocator, "");

        AllTests.allTests(communicator, true);

        return 0;
    }
    
    public static void Main(String[] args)
    {
	int status = 0;
	Ice.Communicator communicator = null;
	
	try
	{
	    Ice.Properties properties = Ice.Util.getDefaultProperties();
	    properties.setProperty("Ice.ThreadPool.Client.Size", "2"); // For nested AMI.
	    
	    communicator = Ice.Util.initialize(ref args);
	    status = run(args, communicator);
	}
	catch(System.Exception ex)
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
