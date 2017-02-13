// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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

public class Server : TestCommon.Application
{
    private static void usage()
    {
        Console.Error.WriteLine("Usage: Server port");
    }

    public override int run(string[] args)
    {
        int port = 0;
        for(int i = 0; i < args.Length; i++)
        {
            if(args[i][0] == '-')
            {
                Console.Error.WriteLine("Server: unknown option `" + args[i] + "'");
                usage();
                return 1;
            }

            if(port != 0)
            {
                Console.Error.WriteLine("Server: only one port can be specified");
                usage();
                return 1;
            }

            try
            {
                port = int.Parse(args[i]);
            }
            catch(FormatException)
            {
                Console.Error.WriteLine("Server: invalid port");
                usage();
                return 1;
            }
        }

        if(port <= 0)
        {
            Console.Error.WriteLine("Server: no port specified");
            usage();
            return 1;
        }

        communicator().getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(port));
        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("TestAdapter");
        Ice.Object obj = new TestI();
        adapter.add(obj, Ice.Util.stringToIdentity("test"));
        adapter.activate();
        communicator().waitForShutdown();
        return 0;
    }

    protected override Ice.InitializationData getInitData(ref string[] args)
    {
        Ice.InitializationData initData = base.getInitData(ref args);
        initData.properties.setProperty("Ice.ServerIdleTime", "120");
        return initData;
    }

    public static int Main(string[] args)
    {
        Server app = new Server();
        return app.runmain(args);
    }
}
