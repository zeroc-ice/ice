// Copyright (c) ZeroC, Inc.

package test.Ice.inactivityTimeout;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Connection;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.Properties;

import test.Ice.inactivityTimeout.Test.TestIntfPrx;
import test.TestHelper;

import java.io.PrintWriter;

public class AllTests {
    static void allTests(TestHelper helper) {
        Communicator communicator = helper.communicator();
        String proxyString = "test: " + helper.getTestEndpoint();
        TestIntfPrx p = TestIntfPrx.uncheckedCast(communicator.stringToProxy(proxyString));

        String proxyString3s = "test: " + helper.getTestEndpoint(1);

        testClientInactivityTimeout(p, helper.getWriter());
        testServerInactivityTimeout(
            helper, proxyString3s, communicator.getProperties(), helper.getWriter());
        testWithOutstandingRequest(p, false, helper.getWriter());
        testWithOutstandingRequest(p, true, helper.getWriter());

        p.shutdown();
    }

    private static void testClientInactivityTimeout(TestIntfPrx p, PrintWriter output) {
        output.write(
            "testing that the client side inactivity timeout shuts down the connection... ");
        output.flush();

        p.ice_ping();
        Connection connection = p.ice_getConnection();
        test(connection != null);

        // The inactivity timeout is 3s on the client side and 5s on the server side. 4 seconds
        // tests the client side.
        try {
            Thread.sleep(4000);
        } catch (InterruptedException ex) {
        }

        p.ice_ping();
        Connection connection2 = p.ice_getConnection();
        test(connection2 != connection);
        output.println("ok");
    }

    private static void testServerInactivityTimeout(
            TestHelper helper, String proxyString, Properties properties, PrintWriter output) {
        output.write(
            "testing that the server side inactivity timeout shuts down the connection... ");
        output.flush();

        // Create a new communicator with the desired properties.
        properties = properties._clone();
        properties.setProperty("Ice.Connection.Client.InactivityTimeout", "5");
        var initData = new InitializationData();
        initData.properties = properties;
        try (var communicator = helper.initialize(initData)) {
            TestIntfPrx p = TestIntfPrx.uncheckedCast(communicator.stringToProxy(proxyString));

            p.ice_ping();
            Connection connection = p.ice_getConnection();
            test(connection != null);

            // The inactivity timeout is 5s on the client side and 3s on the server side. 4 seconds
            // tests the server side.
            try {
                Thread.sleep(4000);
            } catch (InterruptedException ex) {
            }
            p.ice_ping();
            Connection connection2 = p.ice_getConnection();
            test(connection2 != connection);
        }
        output.println("ok");
    }

    private static void testWithOutstandingRequest(
            TestIntfPrx p, boolean oneway, PrintWriter output) {
        String onewayString = oneway ? "one-way" : "two-way";
        output.write(
            "testing the inactivity timeout with an outstanding "
                + onewayString
                + " request... ");
        output.flush();

        if (oneway) {
            p = p.ice_oneway();
        }

        p.ice_ping();
        Connection connection = p.ice_getConnection();
        test(connection != null);

        // The inactivity timeout is 3s on the client side and 5s on the server side; 4 seconds
        // tests only the client-side.
        p.sleep(4000); // two-way blocks for 4 seconds; one-way is non-blocking
        if (oneway) {
            try {
                Thread.sleep(4000);
            } catch (InterruptedException ex) {
            }
        }
        p.ice_ping();
        Connection connection2 = p.ice_getConnection();

        if (oneway) {
            // With a oneway invocation, the inactivity timeout on the client side shut down the
            // first connection.
            test(connection2 != connection);
        } else {
            // With a two-way invocation, the inactivity timeout should not shutdown any connection.
            test(connection2 == connection);
        }
        output.println("ok");
    }

    private static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    private AllTests() {
    }
}
