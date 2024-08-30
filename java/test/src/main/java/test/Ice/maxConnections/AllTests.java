// Copyright (c) ZeroC, Inc.

package test.Ice.maxConnections;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Connection;
import java.io.PrintWriter;
import test.Ice.maxConnections.Test.TestIntfPrx;

public class AllTests {
  static void allTests(test.TestHelper helper) {
    Communicator communicator = helper.communicator();
    String proxyString = "test: " + helper.getTestEndpoint();
    var p = TestIntfPrx.createProxy(communicator, proxyString);

    String proxyStringMax10 = "test: " + helper.getTestEndpoint(1);
    var pMax10 = TestIntfPrx.createProxy(communicator, proxyStringMax10);

    testCreateConnections(p, 100, helper.getWriter());
    testCreateConnectionsWithMax(pMax10, 10, helper.getWriter());
    testCreateConnectionsWithMaxAndRecovery(pMax10, 10, helper.getWriter());

    p.shutdown();
  }

  // Verify that we can create connectionCount connections and send a ping on each connection.
  private static void testCreateConnections(
      TestIntfPrx p, int connectionCount, PrintWriter output) {
    output.write("testing the creation of " + connectionCount + " connections... ");
    output.flush();

    var connectionList = new java.util.ArrayList<Connection>();
    for (int i = 0; i < connectionCount; i++) {
      p = p.ice_connectionId("connection-" + i);
      p.ice_ping();
      connectionList.add(p.ice_getCachedConnection());
    }

    // Close all connections
    for (int i = 0; i < connectionCount; i++) {
      connectionList.get(i).close();
    }

    output.println("ok");
  }

  // Verify that we can create max connections but not more.
  private static void testCreateConnectionsWithMax(TestIntfPrx p, int max, PrintWriter output) {
    output.write(
        "testing the creation of "
            + max
            + " connections with connection lost at "
            + (max + 1)
            + "... ");
    output.flush();

    var connectionList = new java.util.ArrayList<Connection>();
    for (int i = 0; i < max; i++) {
      p = p.ice_connectionId("connection-" + i);
      p.ice_ping();
      connectionList.add(p.ice_getCachedConnection());
    }

    p = p.ice_connectionId("connection-" + max);
    try {
      p.ice_ping();
      test(false);
    } catch (com.zeroc.Ice.ConnectionLostException ex) {
      // expected, the server aborts the connection when MaxConnections is reached
    }

    // Close all connections
    for (int i = 0; i < max; i++) {
      connectionList.get(i).close();
    }

    output.println("ok");
  }

  // Verify that we can create max connections, then connection lost, then recover.
  private static void testCreateConnectionsWithMaxAndRecovery(
      TestIntfPrx p, int max, PrintWriter output) {
    output.write(
        "testing the creation of "
            + max
            + " connections with connection lost at "
            + (max + 1)
            + " then recovery... ");
    output.flush();

    var connectionList = new java.util.ArrayList<Connection>();
    for (int i = 0; i < max; i++) {
      p = p.ice_connectionId("connection-" + i);
      p.ice_ping();
      connectionList.add(p.ice_getCachedConnection());
    }

    p = p.ice_connectionId("connection-" + max);
    try {
      p.ice_ping();
      test(false);
    } catch (com.zeroc.Ice.ConnectionLostException ex) {
      // expected
    }

    // Close one connection
    connectionList.get(0).close();
    connectionList.remove(0);

    // Try again
    p.ice_ping();
    connectionList.add(p.ice_getCachedConnection());

    // Close all connections
    for (int i = 0; i < max; i++) {
      connectionList.get(i).close();
    }

    output.println("ok");
  }

  private static void test(boolean b) {
    if (!b) {
      throw new RuntimeException();
    }
  }
}
