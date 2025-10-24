// Copyright (c) ZeroC, Inc.

package test.Glacier2.router;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Identity;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.ModuleToPackageSliceLoader;
import com.zeroc.Ice.ObjectAdapter;

import test.TestHelper;

public class Server extends TestHelper {

    public void run(String[] args) {
        var initData = new InitializationData();
        initData.sliceLoader = new ModuleToPackageSliceLoader("::Test", "test.Glacier2.router.Test");
        initData.properties = createTestProperties(args);

        try (Communicator communicator = initialize(initData)) {
            communicator
                .getProperties()
                .setProperty("CallbackAdapter.Endpoints", getTestEndpoint(0));
            ObjectAdapter adapter =
                communicator.createObjectAdapter("CallbackAdapter");

            // The test allows "c1" as category.
            adapter.add(new CallbackI(), new Identity("callback", "c1"));

            // The test allows "c2" as category.
            adapter.add(new CallbackI(), new Identity("callback", "c2"));

            // The test rejects "c3" as category.
            adapter.add(new CallbackI(), new Identity("callback", "c3"));

            // The test allows the prefixed userid.
            adapter.add(new CallbackI(), new Identity("callback", "_userid"));
            adapter.activate();
            communicator.waitForShutdown();
        }
    }
}
