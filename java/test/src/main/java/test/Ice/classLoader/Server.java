// Copyright (c) ZeroC, Inc.

package test.Ice.classLoader;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.ModuleToPackageSliceLoader;
import com.zeroc.Ice.Object;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.Util;

import test.TestHelper;

public class Server extends TestHelper {
    public void run(String[] args) {
        var initData = new InitializationData();
        initData.sliceLoader = new ModuleToPackageSliceLoader("::Test", "test.Ice.classLoader.Test");
        initData.properties = createTestProperties(args);
        initData.properties.setProperty("Ice.Default.SlicedFormat", "1");

        try (Communicator communicator = initialize(initData)) {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            Object object = new InitialI(adapter);
            adapter.add(object, Util.stringToIdentity("initial"));
            adapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
