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

public class Client : Test.TestHelper
{
    public override void run(string[] args)
    {
        Ice.InitializationData initData = new Ice.InitializationData();
        initData.observer = Instrumentation.getObserver();

        initData.properties = createTestProperties(ref args);
        initData.properties.setProperty("Ice.RetryIntervals", "0 1 10 1");

        //
        // This test kills connections, so we don't want warnings.
        //
        initData.properties.setProperty("Ice.Warn.Connections", "0");
        using(var communicator = initialize(initData))
        {
            //
            // Configure a second communicator for the invocation timeout
            // + retry test, we need to configure a large retry interval
            // to avoid time-sensitive failures.
            //
            initData.properties.setProperty("Ice.RetryIntervals", "0 1 10000");
            initData.observer = Instrumentation.getObserver();
            using(var communicator2 = initialize(initData))
            {
                Test.RetryPrx retry = AllTests.allTests(communicator, communicator2, "retry:" + getTestEndpoint(0));
                retry.shutdown();
            }
        }
    }

    public static int Main(string[] args)
    {
        return Test.TestDriver.runTest<Client>(args);
    }

}
