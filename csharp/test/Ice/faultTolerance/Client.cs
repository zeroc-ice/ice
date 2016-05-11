// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Client
{
    private static void usage()
    {
        Console.Error.WriteLine("Usage: client port...");
    }

    private static int run(string[] args, Ice.Communicator communicator)
    {
        System.Collections.Generic.List<int> ports = new System.Collections.Generic.List<int>();
        for(int i = 0; i < args.Length; i++)
        {
            int port = 0;
            try
            {
                port = Int32.Parse(args[i]);
            }
            catch(FormatException ex)
            {
                Console.Error.WriteLine(ex);
                return 1;
            }
            ports.Add(port);
        }

        if(ports.Count == 0)
        {
            Console.Error.WriteLine("Client: no ports specified");
            usage();
            return 1;
        }

        AllTests.allTests(communicator, ports);
        return 0;
    }

    public static int Main(string[] args)
    {
        int status = 0;
        Ice.Communicator communicator = null;

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
        catch(Exception ex)
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

        return status;
    }
}
