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

public class Collocated : TestCommon.Application
{
    public override int run(string[] args)
    {
        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("TestAdapter");
        adapter.add(new MetricsI(), Ice.Util.stringToIdentity("metrics"));
        //adapter.activate(); // Don't activate OA to ensure collocation is used.

        communicator().getProperties().setProperty("ControllerAdapter.Endpoints", getTestEndpoint(1));
        Ice.ObjectAdapter controllerAdapter = communicator().createObjectAdapter("ControllerAdapter");
        controllerAdapter.add(new ControllerI(adapter), Ice.Util.stringToIdentity("controller"));
        //controllerAdapter.activate(); // Don't activate OA to ensure collocation is used.

        Test.MetricsPrx metrics = AllTests.allTests(this, _observer);
        metrics.shutdown();
        return 0;
    }

    protected override Ice.InitializationData getInitData(ref string[] args)
    {
        Ice.InitializationData initData = base.getInitData(ref args);
        initData.observer = _observer;
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.metrics");
        initData.properties.setProperty("TestAdapter.Endpoints", getTestEndpoint(initData.properties, 0));
        initData.properties.setProperty("Ice.Admin.Endpoints", "tcp");
        initData.properties.setProperty("Ice.Admin.InstanceName", "client");
        initData.properties.setProperty("Ice.Admin.DelayCreation", "1");
        initData.properties.setProperty("Ice.Warn.Connections", "0");
        initData.properties.setProperty("Ice.Warn.Dispatch", "0");
        initData.properties.setProperty("Ice.MessageSizeMax", "50000");
        initData.properties.setProperty("Ice.Default.Host", "127.0.0.1");

        // TODO: WORKAROUND: .NET connection closure is sometime not detected in a timely fashion
        // and ACM closes the connection first. We increase the default timeout here to prevent
        // the test from failing when this occurs. It usually takes 2 minutes for the connection
        // closure to be detected when this occurs so we set a default timeout of 3 minutes.
        initData.properties.setProperty("Ice.Default.Timeout", "180000");

        return initData;
    }

    public static int Main(string[] args)
    {
        Collocated app = new Collocated();
        return app.runmain(args);
    }

    static CommunicatorObserverI _observer = new CommunicatorObserverI();
}
