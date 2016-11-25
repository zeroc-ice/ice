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

public class Client : TestCommon.Application
{
    public override int run(string[] args)
    {
        Test.MyClassPrx myClass = AllTests.allTests(this);

        Console.Out.Write("testing server shutdown... ");
        Console.Out.Flush();
        myClass.shutdown();
        try
        {
            myClass.opVoid();
            throw new System.Exception();
        }
        catch(Ice.LocalException)
        {
            Console.Out.WriteLine("ok");
        }

        return 0;
    }

    protected override Ice.InitializationData getInitData(ref string[] args)
    {
        Ice.InitializationData initData = base.getInitData(ref args);
        initData.properties.setProperty("Ice.ThreadPool.Client.Size", "2");
        initData.properties.setProperty("Ice.ThreadPool.Client.SizeWarn", "0");
        initData.properties.setProperty("Ice.BatchAutoFlushSize", "100");
        return initData;
    }

    public static int Main(string[] args)
    {
        Client app = new Client();
        return app.runmain(args);
    }
}
