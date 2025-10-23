// Copyright (c) ZeroC, Inc.

package test.Ice.exceptions;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Identity;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.ModuleToPackageSliceLoader;
import com.zeroc.Ice.Object;
import com.zeroc.Ice.ObjectAdapter;

import test.TestHelper;

public class Collocated extends TestHelper {
    public void run(String[] args) {
        InitializationData initData = new InitializationData();
        initData.sliceLoader = new ModuleToPackageSliceLoader("::Test", "test.Ice.exceptions.Test");
        // For this test, we need a dummy logger, otherwise the assertion test will print an error message.
        initData.logger = new DummyLogger();

        initData.properties = createTestProperties(args);
        initData.properties.setProperty("Ice.Warn.Dispatch", "0");
        // No need to set connection properties such as Ice.Warn.Connections or Ice.MessageSizeMax.

        try (Communicator communicator = initialize(initData)) {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            Object object = new ThrowerI();
            adapter.add(object, new Identity("thrower", ""));
            AllTests.allTests(this);
        }
    }
}
