// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
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
