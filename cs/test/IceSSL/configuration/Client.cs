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

public class Client
{
    private static int run(String[] args, Ice.Communicator communicator)
    {
        if(args.Length < 1)
        {
            Console.Error.WriteLine("Usage: client testdir");
            return 1;
        }

        AllTests.allTests(communicator, args[0]);
        
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
