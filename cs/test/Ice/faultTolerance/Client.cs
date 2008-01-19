// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Diagnostics;

public class Client
{
    private static void usage()
    {
        System.Console.Error.WriteLine("Usage: client port...");
    }
    
    private static int run(string[] args, Ice.Communicator communicator)
    {
        System.Collections.ArrayList ports = new System.Collections.ArrayList(args.Length);
        for(int i = 0; i < args.Length; i++)
        {
            int port = 0;
            try
            {
                port = System.Int32.Parse(args[i]);
            }
            catch(System.FormatException ex)
            {
                System.Console.Error.WriteLine(ex);
                return 1;
            }
            ports.Add(port);
        }
        
        if(ports.Count == 0)
        {
            System.Console.Error.WriteLine("Client: no ports specified");
            usage();
            return 1;
        }
        
        AllTests.allTests(communicator, ports);
        return 0;
    }
    
    public static void Main(string[] args)
    {
        int status = 0;
        Ice.Communicator communicator = null;
        
        Debug.Listeners.Add(new ConsoleTraceListener());

        try
        {
            //
            // This test aborts servers, so we don't want warnings.
            //
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = Ice.Util.createProperties(ref args);
            initData.properties.setProperty("Ice.Warn.Connections", "0");

            communicator = Ice.Util.initialize(ref args, initData);
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
            catch(Ice.LocalException ex)
            {
                System.Console.Error.WriteLine(ex);
                status = 1;
            }
        }
        
        if(status != 0)
        {
            System.Environment.Exit(status);
        }
    }
}
