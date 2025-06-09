// Copyright (c) ZeroC, Inc.

package test.Ice.metrics;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.ModuleToPackageSliceLoader;
import com.zeroc.IceMX.UnknownMetricsView;

import test.Ice.metrics.Test.MetricsPrx;
import test.TestHelper;

public class Client extends TestHelper {
    public void run(String[] args) {
        CommunicatorObserverI observer = new CommunicatorObserverI();
        InitializationData initData = new InitializationData();
        initData.sliceLoader = new ModuleToPackageSliceLoader("::Test", "test.Ice.metrics.Test");
        initData.properties = createTestProperties(args);
        initData.properties.setProperty("Ice.Admin.Endpoints", "tcp");
        initData.properties.setProperty("Ice.Admin.InstanceName", "client");
        initData.properties.setProperty("Ice.Admin.DelayCreation", "1");
        initData.properties.setProperty("Ice.Warn.Connections", "0");
        initData.properties.setProperty("Ice.Default.Host", "127.0.0.1");
        // speed up connection establishment tests
        initData.properties.setProperty("Ice.Connection.Client.ConnectTimeout", "1");
        initData.observer = observer;

        try (Communicator communicator = initialize(initData)) {
            MetricsPrx metrics = AllTests.allTests(this, observer);
            metrics.shutdown();
        } catch (UnknownMetricsView ex) {
            throw new RuntimeException(ex);
        }
    }
}
