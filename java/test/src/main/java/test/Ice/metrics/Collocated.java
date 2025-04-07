// Copyright (c) ZeroC, Inc.

package test.Ice.metrics;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.IceMX.UnknownMetricsView;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.Util;

import test.Ice.metrics.Test.MetricsPrx;
import test.TestHelper;

public class Collocated extends TestHelper {
    public void run(String[] args) {
        CommunicatorObserverI observer = new CommunicatorObserverI();
        InitializationData initData = new InitializationData();
        initData.properties = createTestProperties(args);
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.metrics");
        initData.properties.setProperty("Ice.Admin.Endpoints", "tcp");
        initData.properties.setProperty("Ice.Admin.InstanceName", "client");
        initData.properties.setProperty("Ice.Admin.DelayCreation", "1");
        initData.properties.setProperty("Ice.Warn.Connections", "0");
        initData.properties.setProperty("Ice.Warn.Dispatch", "0");
        initData.properties.setProperty("Ice.Default.Host", "127.0.0.1");
        initData.observer = observer;

        try (Communicator communicator = initialize(initData)) {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));

            ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new MetricsI(), Util.stringToIdentity("metrics"));
            // adapter.activate(); // Don't activate OA to ensure collocation is used.

            communicator
                    .getProperties()
                    .setProperty("ForwardingAdapter.Endpoints", getTestEndpoint(1));
            ObjectAdapter forwardingAdapter =
                    communicator.createObjectAdapter("ForwardingAdapter");
            forwardingAdapter.addDefaultServant(adapter.dispatchPipeline(), "");

            communicator
                    .getProperties()
                    .setProperty("ControllerAdapter.Endpoints", getTestEndpoint(2));
            ObjectAdapter controllerAdapter =
                    communicator.createObjectAdapter("ControllerAdapter");
            controllerAdapter.add(
                    new ControllerI(adapter), Util.stringToIdentity("controller"));
            // controllerAdapter.activate(); // Don't activate OA to ensure collocation is used.

            MetricsPrx metrics = AllTests.allTests(this, observer);
            metrics.shutdown();
        } catch (UnknownMetricsView ex) {
            throw new RuntimeException(ex);
        }
    }
}
