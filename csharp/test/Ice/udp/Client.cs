// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Test;
using System;
using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Client : TestCommon.Application
{
    public override int run(string[] args)
    {
        AllTests.allTests(this);

        int num;
        try
        {
            num = args.Length == 1 ? Int32.Parse(args[0]) : 0;
        }
        catch(FormatException)
        {
            num = 0;
        }
        for(int i = 0; i < num; ++i)
        {
            string endpoint = getTestEndpoint(i, "tcp");
            TestIntfPrxHelper.uncheckedCast(communicator().stringToProxy("control:" + endpoint)).shutdown();
        }
        return 0;
    }

    protected override Ice.InitializationData getInitData(ref string[] args)
    {
        Ice.InitializationData initData = base.getInitData(ref args);
        initData.properties.setProperty("Ice.Warn.Connections", "0");
        initData.properties.setProperty("Ice.UDP.SndSize", "16384");
        return initData;
    }

    public static int Main(string[] args)
    {
        Client app = new Client();
        return app.runmain(args);
    }
}
