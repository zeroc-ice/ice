// Copyright (c) ZeroC, Inc.

package test.Ice.idleTimeout;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.ConnectionClose;
import com.zeroc.Ice.ConnectionIdleException;
import com.zeroc.Ice.ConnectionLostException;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.Properties;
import com.zeroc.Ice.Util;
import java.io.PrintWriter;
import test.Ice.idleTimeout.Test.TestIntfPrx;

public class AllTests {
  static void allTests(test.TestHelper helper) {
    Communicator communicator = helper.communicator();
    String proxyString = "test: " + helper.getTestEndpoint();
    var p = TestIntfPrx.createProxy(communicator, proxyString);

    String proxyString3s = "test: " + helper.getTestEndpoint(1);

    testIdleCheckDoesNotAbortConnectionWhenThreadPoolIsExhausted(p, helper.getWriter());
    testConnectionAbortedByIdleCheck(proxyString, communicator.getProperties(), helper.getWriter());
    testEnableDisableIdleCheck(
        true, proxyString3s, communicator.getProperties(), helper.getWriter());
    testEnableDisableIdleCheck(
        false, proxyString3s, communicator.getProperties(), helper.getWriter());
    p.shutdown();
  }

  // The client and server have the same idle timeout (1s) and the server enables connection idle
  // checks (the default).
  // We verify that the server's idle check does not abort the connection as long as this connection
  // receives heartbeats, even when the heartbeats are not read off the connection in a timely
  // manner.
  // To verify this situation, we use an OA with a 1-thread thread pool and use this unique thread
  // for a long synchronous dispatch (sleep).
  private static void testIdleCheckDoesNotAbortConnectionWhenThreadPoolIsExhausted(
      TestIntfPrx p, PrintWriter output) {
    output.write(
        "testing that the idle check does not abort a connection that receives heartbeats... ");
    output.flush();

    // Establish connection.
    p.ice_ping();

    p.sleep(2000); // the implementation in the server sleeps for 2,000ms

    // close connection
    p.ice_getConnection().close(ConnectionClose.GracefullyWithWait);
    output.println("ok");
  }

  // We verify that the idle check aborts the connection when the connection (here server
  // connection) remains idle for longer than idle timeout. Here, the server has an idle timeout of
  // 1s and idle checks enabled.
  // We intentionally misconfigure the client with an idle timeout of 3s to send heartbeats every
  // 1.5s, which is too long to prevent the server from aborting the connection.
  private static void testConnectionAbortedByIdleCheck(
      String proxyString, Properties properties, PrintWriter output) {
    output.write(
        "testing that the idle check aborts a connection that does not receive anything for 1s... ");
    output.flush();

    // Create a new communicator with the desired properties.
    properties = properties._clone();
    properties.setProperty("Ice.Connection.IdleTimeout", "3");
    properties.setProperty("Ice.Warn.Connections", "0");
    var initData = new InitializationData();
    initData.properties = properties;
    try (var communicator = Util.initialize(initData)) {
      var p = TestIntfPrx.createProxy(communicator, proxyString);

      // Establish connection.
      var connection = p.ice_getConnection();
      test(connection != null);

      // The idle check on the server side aborts the connection because it doesn't get a heartbeat
      // in a timely fashion.
      try {
        p.sleep(2000); // the implementation in the server sleeps for 2,000ms
        test(false); // we expect the server to abort the connection after about 1 second.
      } catch (ConnectionLostException e) {
        // Expected
      }
    }
    output.println("ok");
  }

  // Verifies the behavior with the idle check enabled or disabled when the client and the server
  // have mismatched idle timeouts (here: 3s on the server side and 1s on the client side).
  private static void testEnableDisableIdleCheck(
      boolean enabled, String proxyString, Properties properties, PrintWriter output) {
    String enabledString = enabled ? "enabled" : "disabled";
    output.write("testing connection with idle check " + enabledString + "... ");
    output.flush();

    // Create a new communicator with the desired properties.
    properties = properties._clone();
    properties.setProperty("Ice.Connection.IdleTimeout", "1");
    properties.setProperty("Ice.Connection.EnableIdleCheck", enabled ? "1" : "0");
    properties.setProperty("Ice.Warn.Connections", "0");
    var initData = new InitializationData();
    initData.properties = properties;
    try (var communicator = Util.initialize(initData)) {
      var p = TestIntfPrx.createProxy(communicator, proxyString);

      var connection = p.ice_getConnection();
      test(connection != null);

      try {
        p.sleep(2000); // the implementation in the server sleeps for 2,000ms
        test(!enabled);
      } catch (ConnectionIdleException e) {
        test(enabled);
      }
      output.println("ok");
    }
  }

  private static void test(boolean b) {
    if (!b) {
      throw new RuntimeException();
    }
  }
}
