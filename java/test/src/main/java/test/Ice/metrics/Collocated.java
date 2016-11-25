// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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
        com.zeroc.Ice.Communicator communicator = communicator();

        com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
        adapter.add(new MetricsI(), com.zeroc.Ice.Util.stringToIdentity("metrics"));
        //adapter.activate(); // Don't activate OA to ensure collocation is used.

        communicator.getProperties().setProperty("ControllerAdapter.Endpoints", getTestEndpoint(1));
        com.zeroc.Ice.ObjectAdapter controllerAdapter = communicator.createObjectAdapter("ControllerAdapter");
        controllerAdapter.add(new ControllerI(adapter), com.zeroc.Ice.Util.stringToIdentity("controller"));
        //controllerAdapter.activate(); // Don't activate OA to ensure collocation is used.

        try
        {
            MetricsPrx metrics = AllTests.allTests(this, _observer);
            metrics.shutdown();
        }
        catch(com.zeroc.Ice.UserException ex)
        {
            ex.printStackTrace();
            assert(false);
            return 1;
        }
        return 0;
    }

    @Override
    protected GetInitDataResult getInitData(String[] args)
    {
        GetInitDataResult r = super.getInitData(args);
        if(r.initData.properties.getPropertyAsInt("Ice.ThreadInterruptSafe") > 0)
        {
            // With background IO, collocated invocations are
            // dispatched on the server thread pool. This test needs
            // at least 3 threads in the server thread pool to work.
            r.initData.properties.setProperty("Ice.ThreadPool.Server.Size", "3");
        }
        r.initData.properties.setProperty("Ice.Package.Test", "test.Ice.metrics");
        r.initData.properties.setProperty("Ice.Admin.Endpoints", "tcp");
        r.initData.properties.setProperty("Ice.Admin.InstanceName", "client");
        r.initData.properties.setProperty("Ice.Admin.DelayCreation", "1");
        r.initData.properties.setProperty("TestAdapter.Endpoints", getTestEndpoint(r.initData.properties, 0));
        r.initData.properties.setProperty("Ice.Warn.Connections", "0");
        r.initData.properties.setProperty("Ice.Warn.Dispatch", "0");
        r.initData.properties.setProperty("Ice.MessageSizeMax", "50000");
        r.initData.properties.setProperty("Ice.Default.Host", "127.0.0.1");
        r.initData.observer = _observer;
        return r;
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
