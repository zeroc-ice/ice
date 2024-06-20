//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.timeout;

import com.zeroc.Ice.ConnectTimeoutException;
import com.zeroc.Ice.InitializationData;
import java.io.PrintWriter;
import java.util.concurrent.CompletionException;
import test.Ice.timeout.Test.ControllerPrx;
import test.Ice.timeout.Test.TimeoutPrx;

public class AllTests {
  private static void test(boolean b) {
    if (!b) {
      throw new RuntimeException();
    }
  }

  public static void allTests(test.TestHelper helper) {
    ControllerPrx controller =
        ControllerPrx.checkedCast(
            helper.communicator().stringToProxy("controller:" + helper.getTestEndpoint(1)));
    test(controller != null);

    try {
      allTestsWithController(helper, controller);
    } catch (Exception ex) {
      // Ensure the adapter is not in the holding state when an unexpected exception occurs to
      // prevent the test
      // from hanging on exit in case a connection which disables timeouts is still opened.
      controller.resumeAdapter();
      throw ex;
    }
  }

  public static void allTestsWithController(test.TestHelper helper, ControllerPrx controller) {
    com.zeroc.Ice.Communicator communicator = helper.communicator();
    PrintWriter out = helper.getWriter();

    String sref = "timeout:" + helper.getTestEndpoint(0);
    TimeoutPrx timeout = TimeoutPrx.uncheckedCast(communicator.stringToProxy(sref));

    out.print("testing connect timeout... ");
    out.flush();
    {
      //
      // Expect ConnectTimeoutException.
      //
      controller.holdAdapter(-1);
      try {
        timeout.op();
        test(false);
      } catch (com.zeroc.Ice.ConnectTimeoutException ex) {
        // Expected.
      }
      controller.resumeAdapter();
      timeout.op(); // Ensure adapter is active.
    }
    {
      //
      // Expect success.
      //
      var properties = communicator.getProperties()._clone();
      properties.setProperty("Ice.Connection.ConnectTimeout", "-1");
      var initData = new InitializationData();
      initData.properties = properties;
      try (var communicator2 = com.zeroc.Ice.Util.initialize(initData)) {
        TimeoutPrx to = TimeoutPrx.uncheckedCast(communicator2.stringToProxy(sref));
        controller.holdAdapter(100);
        try {
          to.op();
        } catch (ConnectTimeoutException e) {
          test(false);
        }
      }
    }
    out.println("ok");

    out.print("testing invocation timeout... ");
    out.flush();
    {
      com.zeroc.Ice.Connection connection = timeout.ice_getConnection();
      TimeoutPrx to = timeout.ice_invocationTimeout(100);
      test(connection == to.ice_getConnection());
      try {
        to.sleep(1000);
        test(false);
      } catch (com.zeroc.Ice.InvocationTimeoutException ex) {
      }
      timeout.ice_ping();
      to = timeout.ice_invocationTimeout(1000);
      test(connection == to.ice_getConnection());
      try {
        to.sleep(100);
      } catch (com.zeroc.Ice.InvocationTimeoutException ex) {
        test(false);
      }
      test(connection == to.ice_getConnection());
    }
    {
      //
      // Expect InvocationTimeoutException.
      //
      TimeoutPrx to = timeout.ice_invocationTimeout(100);
      try {
        to.sleepAsync(1000).join();
        test(false);
      } catch (CompletionException ex) {
        test(ex.getCause() instanceof com.zeroc.Ice.InvocationTimeoutException);
      }
      timeout.ice_ping();
    }
    {
      //
      // Expect success.
      //
      TimeoutPrx to = timeout.ice_invocationTimeout(1000);
      try {
        to.sleepAsync(100).join();
      } catch (CompletionException ex) {
        test(false);
      }
    }
    out.println("ok");

    out.print("testing close timeout... ");
    out.flush();
    {
      var connection = timeout.ice_getConnection();
      controller.holdAdapter(-1);
      connection.close(com.zeroc.Ice.ConnectionClose.GracefullyWithWait);

      try {
        connection.getInfo(); // getInfo() doesn't throw in the closing state.
      } catch (com.zeroc.Ice.LocalException ex) {
        test(false);
      }

      while (true) {
        try {
          connection.getInfo();
          try {
            Thread.sleep(10);
          } catch (java.lang.InterruptedException ex) {
          }
        } catch (com.zeroc.Ice.ConnectionManuallyClosedException ex) {
          // Expected.
          test(ex.graceful);
          break;
        }
      }
      controller.resumeAdapter();
      timeout.op(); // Ensure adapter is active.
    }
    out.println("ok");

    out.print("testing invocation timeouts with collocated calls... ");
    out.flush();
    {
      communicator.getProperties().setProperty("TimeoutCollocated.AdapterId", "timeoutAdapter");

      com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TimeoutCollocated");
      adapter.activate();

      TimeoutPrx proxy = TimeoutPrx.uncheckedCast(adapter.addWithUUID(new TimeoutI()));
      proxy = proxy.ice_invocationTimeout(100);
      try {
        proxy.sleep(500);
        test(false);
      } catch (com.zeroc.Ice.InvocationTimeoutException ex) {
      }

      try {
        proxy.sleepAsync(500).join();
        test(false);
      } catch (CompletionException ex) {
        test(ex.getCause() instanceof com.zeroc.Ice.InvocationTimeoutException);
      }

      ((TimeoutPrx) proxy.ice_invocationTimeout(-1)).ice_ping();

      TimeoutPrx batchTimeout = proxy.ice_batchOneway();
      batchTimeout.ice_ping();
      batchTimeout.ice_ping();
      batchTimeout.ice_ping();
      proxy.ice_invocationTimeout(-1).sleepAsync(500); // Keep the server thread pool busy.
      try {
        batchTimeout.ice_flushBatchRequestsAsync().join();
        test(false);
      } catch (CompletionException ex) {
        test(ex.getCause() instanceof com.zeroc.Ice.InvocationTimeoutException);
      }

      adapter.destroy();
    }
    out.println("ok");

    controller.shutdown();
  }
}
