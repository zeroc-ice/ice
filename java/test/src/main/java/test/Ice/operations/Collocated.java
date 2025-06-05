// Copyright (c) ZeroC, Inc.

package test.Ice.operations;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.ModuleToPackageSliceLoader;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.ObjectPrx;
import com.zeroc.Ice.Util;

import java.io.PrintWriter;

import test.TestHelper;

public class Collocated extends TestHelper {
    public void run(String[] args) {
        var initData = new InitializationData();
        initData.sliceLoader = new ModuleToPackageSliceLoader("::Test", "test.Ice.operations.Test");
        initData.properties = createTestProperties(args);
        initData.properties.setProperty("Ice.BatchAutoFlushSize", "100");
        // It's possible to have batch oneway requests dispatched after the adapter is deactivated due to thread
        // scheduling so we suppress this warning.
        initData.properties.setProperty("Ice.Warn.Dispatch", "0");

        try (Communicator communicator = initialize(initData)) {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            ObjectPrx prx =
                adapter.add(new MyDerivedClassI(), Util.stringToIdentity("test"));
            // adapter.activate(); // Don't activate OA to ensure collocation is used.

            if (prx.ice_getConnection() != null) {
                throw new RuntimeException();
            }
            AllTests.allTests(this);
            testCollocatedIPv6Invocation(this);
        }
    }

    private static void testCollocatedIPv6Invocation(TestHelper helper) {
        int port = helper.getTestPort(1);
        PrintWriter output = helper.getWriter();
        output.print("testing collocated invocation with normalized IPv6 address... ");
        output.flush();
        try (var communicator = Util.initialize()) {
            communicator
                .getProperties()
                .setProperty("TestAdapter.Endpoints", "tcp -h \"0:0:0:0:0:0:0:1\" -p " + port);
            var adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new MyDerivedClassI(), Util.stringToIdentity("test"));

            var prx = ObjectPrx.createProxy(communicator, "test:tcp -h \"::1\" -p " + port);
            prx.ice_ping();

            prx = ObjectPrx.createProxy(communicator, "test:tcp -h \"0:0:0:0:0:0:0:1\" -p " + port);
            prx.ice_ping();
            output.println();
        }
    }
}
