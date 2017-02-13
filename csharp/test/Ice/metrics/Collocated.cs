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

public class Collocated
{
    private static int run(string[] args, Ice.Communicator communicator)
    {
        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
        adapter.add(new MetricsI(), communicator.stringToIdentity("metrics"));
        //adapter.activate(); // Don't activate OA to ensure collocation is used.

        communicator.getProperties().setProperty("ControllerAdapter.Endpoints", "default -p 12011");
        Ice.ObjectAdapter controllerAdapter = communicator.createObjectAdapter("ControllerAdapter");
        controllerAdapter.add(new ControllerI(adapter), communicator.stringToIdentity("controller"));
        //controllerAdapter.activate(); // Don't activate OA to ensure collocation is used.

        Test.MetricsPrx metrics = AllTests.allTests(communicator, _observer);
        metrics.shutdown();
        return 0;
    }

    public static int Main(string[] args)
    {
        int status = 0;
        Ice.Communicator communicator = null;

#if !COMPACT && !UNITY
        Debug.Listeners.Add(new ConsoleTraceListener());
#endif

        try
        {
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = Ice.Util.createProperties(ref args);
            initData.properties.setProperty("Ice.Package.Test", "test.Ice.metrics");
            initData.properties.setProperty("TestAdapter.Endpoints", "default -p 12010");
            initData.properties.setProperty("Ice.Admin.Endpoints", "tcp");
            initData.properties.setProperty("Ice.Admin.InstanceName", "client");
            initData.properties.setProperty("Ice.Admin.DelayCreation", "1");
            initData.properties.setProperty("Ice.Warn.Connections", "0");
            initData.properties.setProperty("Ice.Warn.Dispatch", "0");
            initData.properties.setProperty("Ice.MessageSizeMax", "50000");
            initData.properties.setProperty("Ice.Default.Host", "127.0.0.1");
#if COMPACT
            //
            // When using Ice for .NET Compact Framework, we need to specify
            // the assembly so that Ice can locate classes and exceptions.
            //
            initData.properties.setProperty("Ice.FactoryAssemblies", "collocated");
#endif
            initData.observer = _observer;
            communicator = Ice.Util.initialize(ref args, initData);
            status = run(args, communicator);
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

        return status;
    }

    static CommunicatorObserverI _observer = new CommunicatorObserverI();
}
