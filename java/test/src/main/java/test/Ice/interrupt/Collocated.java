// Copyright (c) ZeroC, Inc.

package test.Ice.interrupt;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.ModuleToPackageSliceLoader;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.Util;

import test.TestHelper;

public class Collocated extends TestHelper {
    public void run(String[] args) {
        var initData = new InitializationData();
        initData.sliceLoader = new ModuleToPackageSliceLoader("::Test", "test.Ice.interrupt.Test");
        initData.properties = createTestProperties(args);
        // We need to send messages large enough to cause the transport buffers to fill up.
        initData.properties.setProperty("Ice.MessageSizeMax", "20000");
        // opIdempotent raises UnknownException, we disable dispatch warnings to prevent warnings.
        initData.properties.setProperty("Ice.Warn.Dispatch", "0");

        try (Communicator communicator = initialize(initData)) {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            communicator
                .getProperties()
                .setProperty("ControllerAdapter.Endpoints", getTestEndpoint(1));
            communicator.getProperties().setProperty("ControllerAdapter.ThreadPool.Size", "1");

            ObjectAdapter adapter = communicator().createObjectAdapter("TestAdapter");
            ObjectAdapter adapter2 =
                communicator().createObjectAdapter("ControllerAdapter");
            TestControllerI controller = new TestControllerI(adapter);
            adapter.add(new TestI(controller), Util.stringToIdentity("test"));
            // adapter.activate(); // Don't activate OA to ensure collocation is used.
            adapter2.add(controller, Util.stringToIdentity("testController"));
            // adapter2.activate(); // Don't activate OA to ensure collocation is used.
            AllTests.allTests(this);
        } catch (InterruptedException ex) {
            throw new RuntimeException(ex);
        }
    }
}
