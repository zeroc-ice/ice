// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;

public class Client
{
    private static int run(String[] args, Ice.Communicator communicator, 
			   Ice.InitializationData initData)
    {
	Test.MyClassPrx myClass = AllTests.allTests(communicator, initData, false);
	
	Console.Out.Write("testing server shutdown... ");
	Console.Out.Flush();
	myClass.shutdown();
	try
	{
	    myClass.opVoid();
	    throw new System.Exception();
	}
	catch(Ice.LocalException)
	{
	    Console.Out.WriteLine("ok");
	}
	
	return 0;
    }
    
    public static void Main(string[] args)
    {
	int status = 0;
	Ice.Communicator communicator = null;
	
	try
	{
	    Ice.InitializationData initData = new Ice.InitializationData();
	    initData.properties = Ice.Util.createProperties(ref args);
	    initData.properties.setProperty("Ice.ThreadPool.Client.Size", "2"); // For nested AMI.
	    initData.properties.setProperty("Ice.ThreadPool.Client.SizeWarn", "0");

	    //
	    // We must set MessageSizeMax to an explicit values,
	    // because we run tests to check whether
	    // Ice.MemoryLimitException is raised as expected.
	    //
	    initData.properties.setProperty("Ice.MessageSizeMax", "100");

	    //
	    // We don't want connection warnings because of the timeout test.
	    //
	    initData.properties.setProperty("Ice.Warn.Connections", "0");

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
	
        if(status != 0)
        {
            System.Environment.Exit(status);
        }
    }
}
