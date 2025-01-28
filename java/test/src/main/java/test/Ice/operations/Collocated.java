// Copyright (c) ZeroC, Inc.

package test.Ice.operations;

import com.zeroc.Ice.ObjectPrx;
import com.zeroc.Ice.Util;

import java.io.PrintWriter;

public class Collocated extends test.TestHelper {
    public void run(String[] args) {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.operations");
        properties.setProperty("Ice.BatchAutoFlushSize", "100");

        //
        // Its possible to have batch oneway requests dispatched
        // after the adapter is deactivated due to thread
        // scheduling so we suppress this warning.
        //
        properties.setProperty("Ice.Warn.Dispatch", "0");
        try (com.zeroc.Ice.Communicator communicator = initialize(properties)) {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            com.zeroc.Ice.ObjectPrx prx =
                    adapter.add(new MyDerivedClassI(), Util.stringToIdentity("test"));
            // adapter.activate(); // Don't activate OA to ensure collocation is used.

            if (prx.ice_getConnection() != null) {
                throw new RuntimeException();
            }
            AllTests.allTests(this);
            testCollocatedIPv6Invocation(this.getWriter());
        }
    }

    private static void testCollocatedIPv6Invocation(PrintWriter output) {
        output.print("testing collocated invocation with normalized IPv6 address... ");
        output.flush();
        try (var communicator = com.zeroc.Ice.Util.initialize()) {
            communicator
                    .getProperties()
                    .setProperty("TestAdapter.Endpoints", "tcp -h \"0:0:0:0:0:0:0:1\" -p 10000");
            var adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new MyDerivedClassI(), com.zeroc.Ice.Util.stringToIdentity("test"));

            var prx = ObjectPrx.createProxy(communicator, "test:tcp -h \"::1\" -p 10000");
            prx = prx.ice_invocationTimeout(10);
            prx.ice_ping();

            prx = ObjectPrx.createProxy(communicator, "test:tcp -h \"0:0:0:0:0:0:0:1\" -p 10000");
            prx = prx.ice_invocationTimeout(10);
            prx.ice_ping();
            output.println();
        }
    }
}
