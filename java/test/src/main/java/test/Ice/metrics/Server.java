// Copyright (c) ZeroC, Inc.

package test.Ice.metrics;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.Properties;
import com.zeroc.Ice.Util;

import test.TestHelper;

public class Server extends TestHelper {
    public void run(String[] args) {
        Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.retry");
        properties.setProperty("Ice.Admin.Endpoints", "tcp");
        properties.setProperty("Ice.Admin.InstanceName", "server");
        properties.setProperty("Ice.Warn.Connections", "0");
        properties.setProperty("Ice.Warn.Dispatch", "0");
        properties.setProperty("Ice.MessageSizeMax", "50000");
        properties.setProperty("Ice.Default.Host", "127.0.0.1");

        try (Communicator communicator = initialize(properties)) {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new MetricsI(), Util.stringToIdentity("metrics"));
            adapter.activate();

            communicator
                .getProperties()
                .setProperty("ForwardingAdapter.Endpoints", getTestEndpoint(1));
            ObjectAdapter forwardingAdapter =
                communicator.createObjectAdapter("ForwardingAdapter");
            forwardingAdapter.addDefaultServant(adapter.dispatchPipeline(), "");
            forwardingAdapter.activate();

            communicator
                .getProperties()
                .setProperty("ControllerAdapter.Endpoints", getTestEndpoint(2));
            ObjectAdapter controllerAdapter =
                communicator.createObjectAdapter("ControllerAdapter");
            controllerAdapter.add(
                new ControllerI(adapter), Util.stringToIdentity("controller"));
            controllerAdapter.activate();

            serverReady();
            communicator.waitForShutdown();
        }
    }
}
