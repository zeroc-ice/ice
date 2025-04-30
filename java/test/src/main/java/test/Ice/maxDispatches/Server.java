// Copyright (c) ZeroC, Inc.

package test.Ice.maxDispatches;

import com.zeroc.Ice.Identity;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.ModuleToPackageSliceLoader;

import test.TestHelper;

public class Server extends TestHelper {
    public void run(String[] args) {
        var initData = new InitializationData();
        initData.sliceLoader = new ModuleToPackageSliceLoader("::Test", "test.Ice.maxDispatches.Test");
        initData.properties = createTestProperties(args);
         // plenty of threads to handle the requests
        initData.properties.setProperty("Ice.ThreadPool.Server.Size", "10");

        try (var communicator = initialize(initData)) {

            var responder = new ResponderI();
            var testIntf = new TestIntfI(responder);

            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint());
            var adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(testIntf, new Identity("test", ""));
            adapter.activate();

            communicator
                .getProperties()
                .setProperty("ResponderAdapter.Endpoints", getTestEndpoint(1));
            adapter = communicator.createObjectAdapter("ResponderAdapter");
            adapter.add(responder, new Identity("responder", ""));
            adapter.activate();

            communicator
                .getProperties()
                .setProperty("TestAdapterMax10.Endpoints", getTestEndpoint(2));
            communicator
                .getProperties()
                .setProperty("TestAdapterMax10.Connection.MaxDispatches", "10");
            adapter = communicator.createObjectAdapter("TestAdapterMax10");
            adapter.add(testIntf, new Identity("test", ""));
            adapter.activate();

            communicator
                .getProperties()
                .setProperty("TestAdapterMax1.Endpoints", getTestEndpoint(3));
            communicator
                .getProperties()
                .setProperty("TestAdapterMax1.Connection.MaxDispatches", "1");
            adapter = communicator.createObjectAdapter("TestAdapterMax1");
            adapter.add(testIntf, new Identity("test", ""));
            adapter.activate();

            communicator
                .getProperties()
                .setProperty("TestAdapterSerialize.Endpoints", getTestEndpoint(4));
            communicator.getProperties().setProperty("TestAdapterSerialize.ThreadPool.Size", "10");
            communicator
                .getProperties()
                .setProperty("TestAdapterSerialize.ThreadPool.Serialize", "1");
            adapter = communicator.createObjectAdapter("TestAdapterSerialize");
            adapter.add(testIntf, new Identity("test", ""));
            adapter.activate();

            serverReady();
            communicator.waitForShutdown();
        }
    }
}
