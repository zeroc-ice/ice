// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;

public class Collocated
{
    private static int run(String[] args, Ice.Communicator communicator,
			   Ice.InitializationData initData)
    {
        communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -p 12010 -t 2000");
        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
	Ice.Identity id = communicator.stringToIdentity("test");
	adapter.add(new MyDerivedClassI(adapter, id), id);
	adapter.add(new TestCheckedCastI(), communicator.stringToIdentity("context"));
	adapter.activate();

        AllTests.allTests(communicator, initData, true);

        return 0;
    }
    
    public static void Main(String[] args)
    {
	int status = 0;
	Ice.Communicator communicator = null;
	
	try
	{
	    Ice.InitializationData initData = new Ice.InitializationData();
	    initData.properties = Ice.Util.createProperties(ref args);
	    initData.properties.setProperty("Ice.ThreadPool.Client.Size", "2"); // For nested AMI.
	    initData.properties.setProperty("Ice.ThreadPool.Client.SizeWarn", "0");
	    
	    communicator = Ice.Util.initialize(ref args, initData);
	    status = run(args, communicator, initData);
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
