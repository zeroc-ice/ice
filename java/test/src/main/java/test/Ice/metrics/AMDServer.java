// Copyright (c) ZeroC, Inc.

package test.Ice.metrics;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.ModuleToPackageSliceLoader;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.Properties;
import com.zeroc.Ice.Util;

import test.TestHelper;

public class AMDServer extends TestHelper {
    public void run(String[] args) {
        var initData = new InitializationData();
        initData.sliceLoader = new ModuleToPackageSliceLoader("::Test", "test.Ice.metrics.AMD.Test");
        initData.properties = createTestProperties(args);
        initData.properties.setProperty("Ice.Admin.Endpoints", "tcp");
        initData.properties.setProperty("Ice.Admin.InstanceName", "server");
        initData.properties.setProperty("Ice.Warn.Connections", "0");
        initData.properties.setProperty("Ice.Warn.Dispatch", "0");
        initData.properties.setProperty("Ice.MessageSizeMax", "50000");

        try (Communicator communicator = initialize(initData)) {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new AMDMetricsI(), Util.stringToIdentity("metrics"));
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
