// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;
using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Server
{
    private static void usage()
    {
        System.Console.Error.WriteLine("Usage: Server port");
    }

    private static int run(string[] args, Ice.Communicator communicator)
    {
        int port = 0;
        for(int i = 0; i < args.Length; i++)
        {
            if(args[i][0] == '-')
            {
                System.Console.Error.WriteLine("Server: unknown option `" + args[i] + "'");
                usage();
                return 1;
            }

            if(port != 0)
            {
                System.Console.Error.WriteLine("Server: only one port can be specified");
                usage();
                return 1;
            }

            try
            {
                port = System.Int32.Parse(args[i]);
            }
            catch(System.FormatException)
            {
                System.Console.Error.WriteLine("Server: invalid port");
                usage();
                return 1;
            }
        }

        if(port <= 0)
        {
            System.Console.Error.WriteLine("Server: no port specified");
            usage();
            return 1;
        }

        communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -p " + port + ":udp");
        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
        Ice.Object obj = new TestI();
        adapter.add(obj, communicator.stringToIdentity("test"));
        adapter.activate();
        communicator.waitForShutdown();
        return 0;
    }

    public static int Main(string[] args)
    {
        int status = 0;
        Ice.Communicator communicator = null;

        try
        {
            //
            // In this test, we need longer server idle time,
            // otherwise our test servers may time out before they are
            // used in the test.
            //
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = Ice.Util.createProperties(ref args);
            initData.properties.setProperty("Ice.ServerIdleTime", "120"); // Two minutes

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

        return status;
    }
}
