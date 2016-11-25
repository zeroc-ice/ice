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
        PluginI plugin = new PluginI(communicator());
        plugin.initialize();
        communicator().getPluginManager().addPlugin("Test", plugin);
        Test.BackgroundPrx background = AllTests.allTests(this);
        background.shutdown();
        return 0;
    }

    protected override Ice.InitializationData getInitData(ref string[] args)
    {
        Ice.InitializationData initData = base.getInitData(ref args);
        //
        // For this test, we want to disable retries.
        //
        initData.properties.setProperty("Ice.RetryIntervals", "-1");

        //
        // This test kills connections, so we don't want warnings.
        //
        initData.properties.setProperty("Ice.Warn.Connections", "0");

        initData.properties.setProperty("Ice.MessageSizeMax", "50000");

        // This test relies on filling the TCP send/recv buffer, so
        // we rely on a fixed value for these buffers.
        initData.properties.setProperty("Ice.TCP.SndSize", "50000");

        //
        // Setup the test transport plug-in.
        //
        string defaultProtocol = initData.properties.getPropertyWithDefault("Ice.Default.Protocol", "tcp");
        initData.properties.setProperty("Ice.Default.Protocol", "test-" + defaultProtocol);
        return initData;
    }

    public static int Main(string[] args)
    {
        Client app = new Client();
        return app.runmain(args);
    }
}
