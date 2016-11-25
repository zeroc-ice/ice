// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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

public class Client : TestCommon.Application
{
    public override int run(string[] args)
    {
        Test.MetricsPrx metrics = AllTests.allTests(this, _observer);
        metrics.shutdown();
        return 0;
    }

    protected override Ice.InitializationData getInitData(ref string[] args)
    {
        Ice.InitializationData initData = base.getInitData(ref args);
        initData.observer = _observer;
        initData.properties.setProperty("Ice.Admin.Endpoints", "tcp");
        initData.properties.setProperty("Ice.Admin.InstanceName", "client");
        initData.properties.setProperty("Ice.Admin.DelayCreation", "1");
        initData.properties.setProperty("Ice.Warn.Connections", "0");
        initData.properties.setProperty("Ice.MessageSizeMax", "50000");
        initData.properties.setProperty("Ice.Default.Host", "127.0.0.1");
        return initData;
    }

    public static int Main(string[] args)
    {
        Client app = new Client();
        return app.runmain(args);
    }

    static CommunicatorObserverI _observer = new CommunicatorObserverI();
}
