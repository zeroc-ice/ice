// Copyright (c) ZeroC, Inc.

package test.Ice.maxConnections;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Connection;
import com.zeroc.Ice.ConnectionLostException;

import test.Ice.maxConnections.Test.TestIntfPrx;
import test.TestHelper;

import java.io.PrintWriter;
import java.util.ArrayList;

public class AllTests {
    static void allTests(TestHelper helper) {
        Communicator communicator = helper.communicator();
        String proxyString = "test: " + helper.getTestEndpoint();
        var p = TestIntfPrx.createProxy(communicator, proxyString);

        String proxyStringMax10 = "test: " + helper.getTestEndpoint(1);
        var pMax10 = TestIntfPrx.createProxy(communicator, proxyStringMax10);

        Runnable postCloseDelay = null;
        // When the transport is WS or WSS, we need to wait a little bit: the server closes
        // the connection after it gets a transport frame from the client.
        if (helper.getTestProtocol().startsWith("ws")) {
            postCloseDelay =
                    () -> {
                        try {
                            Thread.sleep(50);
                        } catch (InterruptedException ex) {
                            // ignore
                        }
                    };
        }

        testCreateConnections(p, 100, helper.getWriter(), postCloseDelay);
        testCreateConnectionsWithMax(pMax10, 10, helper.getWriter(), postCloseDelay);
        testCreateConnectionsWithMaxAndRecovery(pMax10, 10, helper.getWriter(), postCloseDelay);

        p.shutdown();
    }

    // Verify that we can create connectionCount connections and send a ping on each connection.
    private static void testCreateConnections(
            TestIntfPrx p, int connectionCount, PrintWriter output, Runnable postCloseDelay) {
        output.write("testing the creation of " + connectionCount + " connections... ");
        output.flush();

        var connectionList = new ArrayList<Connection>();
        for (int i = 0; i < connectionCount; i++) {
            p = p.ice_connectionId("connection-" + i);
            p.ice_ping();
            connectionList.add(p.ice_getCachedConnection());
        }

        // Close all connections
        for (int i = 0; i < connectionCount; i++) {
            connectionList.get(i).close();
        }

        if (postCloseDelay != null) {
            postCloseDelay.run();
        }

        output.println("ok");
    }

    // Verify that we can create max connections but not more.
    private static void testCreateConnectionsWithMax(
            TestIntfPrx p, int max, PrintWriter output, Runnable postCloseDelay) {
        output.write(
                "testing the creation of "
                        + max
                        + " connections with connection lost at "
                        + (max + 1)
                        + "... ");
        output.flush();

        var connectionList = new ArrayList<Connection>();
        for (int i = 0; i < max; i++) {
            p = p.ice_connectionId("connection-" + i);
            p.ice_ping();
            connectionList.add(p.ice_getCachedConnection());
        }

        p = p.ice_connectionId("connection-" + max);
        try {
            p.ice_ping();
            test(false);
        } catch (ConnectionLostException ex) {
            // expected, the server aborts the connection when MaxConnections is reached
        }

        // Close all connections
        for (int i = 0; i < max; i++) {
            connectionList.get(i).close();
        }

        if (postCloseDelay != null) {
            postCloseDelay.run();
        }

        output.println("ok");
    }

    // Verify that we can create max connections, then connection lost, then recover.
    private static void testCreateConnectionsWithMaxAndRecovery(
            TestIntfPrx p, int max, PrintWriter output, Runnable postCloseDelay) {
        output.write(
                "testing the creation of "
                        + max
                        + " connections with connection lost at "
                        + (max + 1)
                        + " then recovery... ");
        output.flush();

        var connectionList = new ArrayList<Connection>();
        for (int i = 0; i < max; i++) {
            p = p.ice_connectionId("connection-" + i);
            p.ice_ping();
            connectionList.add(p.ice_getCachedConnection());
        }

        p = p.ice_connectionId("connection-" + max);
        try {
            p.ice_ping();
            test(false);
        } catch (ConnectionLostException ex) {
            // expected
        }

        // Close one connection
        connectionList.get(0).close();
        connectionList.remove(0);

        if (postCloseDelay != null) {
            postCloseDelay.run();
        } else {
            // We need to wait a tiny bit to let the server remove the connection from its incoming
            // connection factory.
            try {
                Thread.sleep(10);
            } catch (InterruptedException ex) {
                // ignore
            }
        }

        // Try again
        p.ice_ping();
        connectionList.add(p.ice_getCachedConnection());

        // Close all connections
        for (int i = 0; i < max; i++) {
            connectionList.get(i).close();
        }

        if (postCloseDelay != null) {
            postCloseDelay.run();
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
