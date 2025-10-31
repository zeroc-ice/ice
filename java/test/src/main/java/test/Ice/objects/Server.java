// Copyright (c) ZeroC, Inc.

package test.Ice.objects;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Identity;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.ModuleToPackageSliceLoader;
import com.zeroc.Ice.ObjectAdapter;

import test.TestHelper;

public class Server extends TestHelper {
    public void run(String[] args) {
        var initData = new InitializationData();
        initData.sliceLoader = new ModuleToPackageSliceLoader("::Test", "test.Ice.objects.Test");
        initData.properties = createTestProperties(args);
        initData.properties.setProperty("Ice.Warn.Dispatch", "0");

        try (Communicator communicator = initialize(initData)) {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));

            ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new InitialI(adapter), new Identity("initial", ""));
            adapter.add(new F2I(), new Identity("F21", ""));
            adapter.add(
                new UnexpectedObjectExceptionTestI(),
                new Identity("uoet", ""));
            adapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
