// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;

public class Server
{
    private static int run(string[] args, Ice.Communicator communicator)
    {
        communicator.getProperties().setProperty("TestAdapter.Endpoints", "tcp -p 12010");
        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
        Ice.Identity id = communicator.stringToIdentity("factory");
        adapter.add(new ServerFactoryI(), id);
        adapter.activate();

        communicator.waitForShutdown();
        return 0;
    }
    
    public static void Main(string[] args)
    {
        int status = 0;
        Ice.Communicator communicator = null;
        
        Debug.Listeners.Add(new ConsoleTraceListener());

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
