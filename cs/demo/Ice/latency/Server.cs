// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
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
        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("Latency");
        Ice.Object @object = new Ping();
        adapter.add(@object, Ice.Util.stringToIdentity("ping"));
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
            Ice.Properties properties = Ice.Util.createProperties(ref args);
            properties.load("config");
            communicator = Ice.Util.initializeWithProperties(ref args, properties);
            status = run(args, communicator);
        }
        catch(System.Exception ex)
        {
	    Console.Error.WriteLine(ex);
            status = 1;
        }
        finally
        {
            if(communicator != null)
            {
                communicator.destroy();
            }
        }
        
        System.Environment.Exit(status);
    }
}
