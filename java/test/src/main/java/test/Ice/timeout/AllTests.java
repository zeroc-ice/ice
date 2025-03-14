// Copyright (c) ZeroC, Inc.

package test.Ice.timeout;

import com.zeroc.Ice.ConnectTimeoutException;
import com.zeroc.Ice.InitializationData;

import test.Ice.timeout.Test.ControllerPrx;
import test.Ice.timeout.Test.TimeoutPrx;

import java.io.PrintWriter;
import java.util.concurrent.CompletionException;

public class AllTests {
    private static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    private static com.zeroc.Ice.Connection connect(com.zeroc.Ice.ObjectPrx prx) {
        int nRetry = 10;
        while (--nRetry > 0) {
            try {
                prx.ice_getConnection();
                break;
            } catch (com.zeroc.Ice.ConnectTimeoutException ex) {
                // Can sporadically occur with slow machines
            }
        }
        return prx.ice_getConnection(); // Establish connection
    }

    public static void allTests(test.TestHelper helper) {
        var controller =
                ControllerPrx.createProxy(
                        helper.communicator(), "controller:" + helper.getTestEndpoint(1));

        // Make sure the controller is connected before we proceed.
        connect(controller);

        try {
            allTestsWithController(helper, controller);
        } catch (Exception ex) {
            // Ensure the adapter is not in the holding state when an unexpected exception occurs to
            // prevent the test from hanging on exit in case a connection which disables timeouts is
            // still opened.
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
                TimeoutPrx.uncheckedCast(timeout.ice_connectionId("connection-1")).op();
                test(false);
            } catch (com.zeroc.Ice.ConnectTimeoutException ex) {
                // Expected.
            }

            try {
                TimeoutPrx.uncheckedCast(timeout.ice_connectionId("connection-2")).op();
                test(false);
            } catch (com.zeroc.Ice.ConnectTimeoutException ex) {
                // Expected.
            }
            controller.resumeAdapter();
            // Retrying with a new connection.
            TimeoutPrx.uncheckedCast(timeout.ice_connectionId("connection-3")).op();
        }
        {
            //
            // Expect success.
            //
            var properties = communicator.getProperties()._clone();
            properties.setProperty("Ice.Connection.Client.ConnectTimeout", "-1");
            var initData = new InitializationData();
            initData.properties = properties;
            try (var communicator2 = helper.initialize(initData)) {
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
            // This test wants to call some local methods while our connection is in the `Closing`
            // state, before it eventually transitions to the `Closed` state due to hitting the
            // close timeout.
            //
            // However, in Java `close` blocks until the connection is closed. So, in order to
            // access the `Closing` state, we initiate the close in a separate thread, wait 50ms to
            // let the thread start the closure process, and hope that we're in the `Closing` state
            // by then.

            // Get the connection, and put the OA in the `Hold` state.
            var connection = connect(timeout);
            controller.holdAdapter(-1);

            // Initiate the connection closure.
            var closureThread =
                    new Thread(
                            () -> {
                                try {
                                    connection.close();
                                    test(false);
                                } catch (com.zeroc.Ice.CloseTimeoutException ex) {
                                    // Expected.
                                }
                            });

            closureThread.start();
            try {
                Thread.sleep(50);
            } catch (InterruptedException ex) {
            }

            // We set a connect timeout of '1s', so the connection should still be useable here.
            try {
                connection.getInfo(); // getInfo() doesn't throw in the closing state.
            } catch (com.zeroc.Ice.LocalException ex) {
                test(false);
            }

            try {
                closureThread.join(); // Ensure the connection closure completed.
            } catch (InterruptedException ex) {
            }

            try {
                connection.getInfo();
            } catch (com.zeroc.Ice.CloseTimeoutException ex) {
                // Expected.
            }

            try {
                closureThread.join(); // Ensure the connection closure thread completed.
            } catch (InterruptedException ex) {
            }

            controller.resumeAdapter();
            timeout.op(); // Ensure adapter is active.
        }
        out.println("ok");

        out.print("testing invocation timeouts with collocated calls... ");
        out.flush();
        {
            communicator
                    .getProperties()
                    .setProperty("TimeoutCollocated.AdapterId", "timeoutAdapter");

            com.zeroc.Ice.ObjectAdapter adapter =
                    communicator.createObjectAdapter("TimeoutCollocated");
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
