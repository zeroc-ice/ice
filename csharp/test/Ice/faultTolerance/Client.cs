// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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

public class Client : TestCommon.Application
{
    private static void usage()
    {
        Console.Error.WriteLine("Usage: client port...");
    }

    public override int run(string[] args)
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

        AllTests.allTests(this, ports);
        return 0;
    }

    protected override Ice.InitializationData getInitData(ref string[] args)
    {
        Ice.InitializationData initData = base.getInitData(ref args);
        initData.properties.setProperty("Ice.Warn.Connections", "0");
        return initData;
    }

    public static int Main(string[] args)
    {
        Client app = new Client();
        return app.runmain(args);
    }
}
