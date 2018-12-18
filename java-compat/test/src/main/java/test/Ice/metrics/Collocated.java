// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.metrics;

import test.Ice.metrics.Test.MetricsPrx;

public class Collocated extends test.TestHelper
{
    public void run(String[] args)
    {
        CommunicatorObserverI observer = new CommunicatorObserverI();

        Ice.InitializationData initData = new Ice.InitializationData();
        initData.properties = createTestProperties(args);
        if(initData.properties.getPropertyAsInt("Ice.ThreadInterruptSafe") > 0)
        {
            // With background IO, collocated invocations are
            // dispatched on the server thread pool. This test needs
            // at least 3 threads in the server thread pool to work.
            initData.properties.setProperty("Ice.ThreadPool.Server.Size", "3");
        }
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.metrics");
        initData.properties.setProperty("Ice.Admin.Endpoints", "tcp");
        initData.properties.setProperty("Ice.Admin.InstanceName", "client");
        initData.properties.setProperty("Ice.Admin.DelayCreation", "1");
        initData.properties.setProperty("Ice.Warn.Connections", "0");
        initData.properties.setProperty("Ice.Warn.Dispatch", "0");
        initData.properties.setProperty("Ice.Default.Host", "127.0.0.1");
        initData.observer = observer;

        try(Ice.Communicator communicator = initialize(initData))
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new MetricsI(), Ice.Util.stringToIdentity("metrics"));
            //adapter.activate(); // Don't activate OA to ensure collocation is used.

            communicator.getProperties().setProperty("ControllerAdapter.Endpoints", getTestEndpoint(1));
            Ice.ObjectAdapter controllerAdapter = communicator.createObjectAdapter("ControllerAdapter");
            controllerAdapter.add(new ControllerI(adapter), Ice.Util.stringToIdentity("controller"));
            //controllerAdapter.activate(); // Don't activate OA to ensure collocation is used.

            MetricsPrx metrics = AllTests.allTests(this, observer);
            metrics.shutdown();
        }
        catch(Exception ex)
        {
            throw new RuntimeException(ex);
        }
    }
}
