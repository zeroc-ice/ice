//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.metrics;

import test.Ice.metrics.Test.MetricsPrx;

public class Client extends test.TestHelper
{
    public void run(String[] args)
    {
        CommunicatorObserverI observer = new CommunicatorObserverI();

        Ice.InitializationData initData = new Ice.InitializationData();
        initData.properties = createTestProperties(args);
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.metrics");
        initData.properties.setProperty("Ice.Admin.Endpoints", "tcp");
        initData.properties.setProperty("Ice.Admin.InstanceName", "client");
        initData.properties.setProperty("Ice.Admin.DelayCreation", "1");
        initData.properties.setProperty("Ice.Warn.Connections", "0");
        initData.properties.setProperty("Ice.Default.Host", "127.0.0.1");
        initData.observer = observer;

        try(Ice.Communicator communicator = initialize(initData))
        {
            MetricsPrx metrics = AllTests.allTests(this, observer);
            metrics.shutdown();
        }
        catch(Exception ex)
        {
            throw new RuntimeException(ex);
        }
    }
}
