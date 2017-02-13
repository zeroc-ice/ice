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

public class Collocated : TestCommon.Application
{
    public override int run(string[] args)
    {
        //
        // Configure a second communicator for the invocation timeout
        // + retry test, we need to configure a large retry interval
        // to avoid time-sensitive failures.
        //
        Ice.InitializationData initData2 = new Ice.InitializationData();
        initData2.properties = communicator().getProperties().ice_clone_();
        initData2.properties.setProperty("Ice.RetryIntervals", "0 1 10000");
        initData2.observer = Instrumentation.getObserver();
        Ice.Communicator communicator2 = Ice.Util.initialize(initData2);

        communicator().createObjectAdapter("").add(new RetryI(), Ice.Util.stringToIdentity("retry"));
        communicator2.createObjectAdapter("").add(new RetryI(), Ice.Util.stringToIdentity("retry"));

        Test.RetryPrx retry = AllTests.allTests(this, communicator2, "retry");
        retry.shutdown();
        return 0;
    }

    protected override Ice.InitializationData getInitData(ref string[] args)
    {
        Ice.InitializationData initData = base.getInitData(ref args);
        initData.observer = Instrumentation.getObserver();

        initData.properties.setProperty("Ice.RetryIntervals", "0 1 10 1");
        initData.properties.setProperty("Ice.Warn.Dispatch", "0");

        //
        // This test kills connections, so we don't want warnings.
        //
        initData.properties.setProperty("Ice.Warn.Connections", "0");
        return initData;
    }

    public static int Main(string[] args)
    {
        Collocated app = new Collocated();
        return app.runmain(args);
    }
}
