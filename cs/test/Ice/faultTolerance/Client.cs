// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

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
                SupportClass.WriteStackTrace(ex, System.Console.Error);
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
        
        try
        {
            communicator = Ice.Util.initialize(ref args);
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
        
        System.Environment.Exit(status);
    }
}
