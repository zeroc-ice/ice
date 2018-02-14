// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.metrics;

import test.Ice.metrics.Test.MetricsPrx;

public class Collocated extends test.Util.Application
{
    @Override
    public int run(String[] args)
    {
        Ice.Communicator communicator = communicator();

        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
        adapter.add(new MetricsI(), Ice.Util.stringToIdentity("metrics"));
        //adapter.activate(); // Don't activate OA to ensure collocation is used.

        communicator.getProperties().setProperty("ControllerAdapter.Endpoints", getTestEndpoint(1));
        Ice.ObjectAdapter controllerAdapter = communicator.createObjectAdapter("ControllerAdapter");
        controllerAdapter.add(new ControllerI(adapter), Ice.Util.stringToIdentity("controller"));
        //controllerAdapter.activate(); // Don't activate OA to ensure collocation is used.

        try
        {
            MetricsPrx metrics = AllTests.allTests(this, _observer);
            metrics.shutdown();
        }
        catch(Ice.UserException ex)
        {
            ex.printStackTrace();
            assert(false);
            return 1;
        }
        return 0;
    }

    @Override
    protected Ice.InitializationData getInitData(Ice.StringSeqHolder argsH)
    {
        Ice.InitializationData initData = super.getInitData(argsH);
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
        initData.properties.setProperty("TestAdapter.Endpoints", getTestEndpoint(initData.properties, 0));
        initData.properties.setProperty("Ice.Warn.Connections", "0");
        initData.properties.setProperty("Ice.Warn.Dispatch", "0");
        initData.properties.setProperty("Ice.MessageSizeMax", "50000");
        initData.properties.setProperty("Ice.Default.Host", "127.0.0.1");
        initData.observer = _observer;
        return initData;
    }

    public static void main(String[] args)
    {
        Collocated app = new Collocated();
        int result = app.main("Collocated", args);
        System.gc();
        System.exit(result);
    }

    private CommunicatorObserverI _observer = new CommunicatorObserverI();
}
