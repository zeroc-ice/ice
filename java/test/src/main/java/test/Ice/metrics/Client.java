// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.metrics;

import test.Ice.metrics.Test.MetricsPrx;

public class Client extends test.TestHelper
{
    public void run(String[] args)
    {
        CommunicatorObserverI observer = new CommunicatorObserverI();
        com.zeroc.Ice.InitializationData initData = new com.zeroc.Ice.InitializationData();
        initData.properties = createTestProperties(args);
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.metrics");
        initData.properties.setProperty("Ice.Admin.Endpoints", "tcp");
        initData.properties.setProperty("Ice.Admin.InstanceName", "client");
        initData.properties.setProperty("Ice.Admin.DelayCreation", "1");
        initData.properties.setProperty("Ice.Warn.Connections", "0");
        initData.properties.setProperty("Ice.Default.Host", "127.0.0.1");
        initData.observer = observer;

        try(com.zeroc.Ice.Communicator communicator = initialize(initData))
        {
            MetricsPrx metrics = AllTests.allTests(this, observer);
            metrics.shutdown();
        }
        catch(com.zeroc.IceMX.UnknownMetricsView ex)
        {
            throw new RuntimeException(ex);
        }
    }
}
