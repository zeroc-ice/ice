// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public class Server
{
    private static int run(string[] args, Ice.Communicator communicator)
    {
        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("Throughput");
        Ice.Object obj = new ThroughputI();
        adapter.add(obj, Ice.Util.stringToIdentity("throughput"));
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
            Ice.Properties properties = Ice.Util.createProperties();
            properties.load("config");
            communicator = Ice.Util.initializeWithProperties(ref args, properties);
            status = run(args, communicator);
        }
        catch(System.Exception ex)
        {
	    System.Console.Error.WriteLine(ex);
            status = 1;
        }
        
	if(communicator != null)
	{
	    try
	    {
		communicator.destroy();
	    }
	    catch(System.Exception ex)
	    {
		System.Console.Error.WriteLine(ex);
		status = 1;
	    }
	}
        
        System.Environment.Exit(status);
    }
}
