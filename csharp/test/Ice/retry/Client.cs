// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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

public class Client
{
    public static int run(string[] args, Ice.Communicator communicator, Ice.Communicator communicator2)
    {
        Test.RetryPrx retry = AllTests.allTests(communicator, communicator2, "retry:default -p 12010");
        retry.shutdown();
        return 0;
    }

    public static int Main(string[] args)
    {
        int status = 0;
        Ice.Communicator communicator = null;
        Ice.Communicator communicator2 = null;

        try
        {
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = Ice.Util.createProperties(ref args);
            initData.observer = Instrumentation.getObserver();

            initData.properties.setProperty("Ice.RetryIntervals", "0 1 10 1");

            //
            // This test kills connections, so we don't want warnings.
            //
            initData.properties.setProperty("Ice.Warn.Connections", "0");

            communicator = Ice.Util.initialize(ref args, initData);

            //
            // Configure a second communicator for the invocation timeout
            // + retry test, we need to configure a large retry interval
            // to avoid time-sensitive failures.
            //
            Ice.InitializationData initData2 = new Ice.InitializationData();
            initData2.properties = initData.properties.ice_clone_();
            initData2.properties.setProperty("Ice.RetryIntervals", "0 1 10000");
            initData2.observer = Instrumentation.getObserver();
            communicator2 = Ice.Util.initialize(initData2);

            status = run(args, communicator, communicator2);
        }
        catch(System.Exception ex)
        {
            Console.Error.WriteLine(ex);
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
                Console.Error.WriteLine(ex);
                status = 1;
            }
        }

        if(communicator2 != null)
        {
            try
            {
                communicator2.destroy();
            }
            catch(Ice.LocalException ex)
            {
                Console.Error.WriteLine(ex);
                status = 1;
            }
        }

        return status;
    }
}
