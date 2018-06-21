// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.timeout;

import java.io.PrintWriter;
import java.util.concurrent.CompletionException;

import test.Ice.timeout.Test.TimeoutPrx;
import test.Ice.timeout.Test.ControllerPrx;

public class AllTests
{
    private static void test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    private static class Callback
    {
        Callback()
        {
            _called = false;
        }

        public synchronized void check()
        {
            while(!_called)
            {
                try
                {
                    wait();
                }
                catch(InterruptedException ex)
                {
                }
            }

            _called = false;
        }

        public synchronized void called()
        {
            assert(!_called);
            _called = true;
            notify();
        }

        private boolean _called;
    }

    public static com.zeroc.Ice.Connection connect(com.zeroc.Ice.ObjectPrx prx)
    {
        int nRetry = 10;
        while(--nRetry > 0)
        {
            try
            {
                prx.ice_getConnection();
                break;
            }
            catch(com.zeroc.Ice.ConnectTimeoutException ex)
            {
                // Can sporadically occur with slow machines
            }
        }
        return prx.ice_getConnection(); // Establish connection
    }

    public static void allTests(test.TestHelper helper)
    {
        com.zeroc.Ice.Communicator communicator = helper.communicator();
        PrintWriter out = helper.getWriter();

        String sref = "timeout:" + helper.getTestEndpoint(0);
        com.zeroc.Ice.ObjectPrx obj = communicator.stringToProxy(sref);
        test(obj != null);

        int mult = 1;
        if(!communicator.getProperties().getPropertyWithDefault("Ice.Default.Protocol", "tcp").equals("tcp") ||
           helper.isAndroid())
        {
            mult = 4;
        }

        TimeoutPrx timeout = TimeoutPrx.checkedCast(obj);
        test(timeout != null);

        ControllerPrx controller = ControllerPrx.checkedCast(
           communicator.stringToProxy("controller:" + helper.getTestEndpoint(1)));
        test(controller != null);

        out.print("testing connect timeout... ");
        out.flush();
        {
            //
            // Expect ConnectTimeoutException.
            //
            TimeoutPrx to = timeout.ice_timeout(100 * mult);
            controller.holdAdapter(-1);
            try
            {
                to.op();
                test(false);
            }
            catch(com.zeroc.Ice.ConnectTimeoutException ex)
            {
                // Expected.
            }
            controller.resumeAdapter();
            timeout.op(); // Ensure adapter is active.
        }
        {
            //
            // Expect success.
            //
            TimeoutPrx to = timeout.ice_timeout(2000 * mult);
            controller.holdAdapter(100 * mult);
            try
            {
                to.op();
            }
            catch(com.zeroc.Ice.ConnectTimeoutException ex)
            {
                test(false);
            }
        }
        out.println("ok");

        // The sequence needs to be large enough to fill the write/recv buffers
        int bufSize = 2000000;
        byte[] seq = new byte[bufSize];

        out.print("testing connection timeout... ");
        out.flush();
        {
            //
            // Expect TimeoutException.
            //
            TimeoutPrx to = timeout.ice_timeout(250);
            connect(to);
            controller.holdAdapter(-1);
            try
            {
                to.sendData(seq);
                test(false);
            }
            catch(com.zeroc.Ice.TimeoutException ex)
            {
                // Expected.
            }
            controller.resumeAdapter();
            timeout.op(); // Ensure adapter is active.
        }
        {
            //
            // Expect success.
            //
            TimeoutPrx to = timeout.ice_timeout(1000 * mult);
            controller.holdAdapter(100 * mult);
            try
            {
                to.sendData(new byte[1000000]);
            }
            catch(com.zeroc.Ice.TimeoutException ex)
            {
                test(false);
            }
        }
        out.println("ok");

        out.print("testing invocation timeout... ");
        out.flush();
        {
            com.zeroc.Ice.Connection connection = obj.ice_getConnection();
            TimeoutPrx to = timeout.ice_invocationTimeout(100);
            test(connection == to.ice_getConnection());
            try
            {
                to.sleep(500 * mult);
                test(false);
            }
            catch(com.zeroc.Ice.InvocationTimeoutException ex)
            {
            }
            obj.ice_ping();
            to = TimeoutPrx.checkedCast(obj.ice_invocationTimeout(500 * mult));
            test(connection == to.ice_getConnection());
            try
            {
                to.sleep(100 * mult);
            }
            catch(com.zeroc.Ice.InvocationTimeoutException ex)
            {
                test(false);
            }
            test(connection == to.ice_getConnection());
        }
        {
            //
            // Expect InvocationTimeoutException.
            //
            TimeoutPrx to = timeout.ice_invocationTimeout(100);
            Callback cb = new Callback();
            to.sleepAsync(500 * mult).whenComplete((result, ex) ->
                {
                    test(ex != null && ex instanceof com.zeroc.Ice.TimeoutException);
                    cb.called();
                });
            cb.check();
            obj.ice_ping();
        }
        {
            //
            // Expect success.
            //
            TimeoutPrx to = timeout.ice_invocationTimeout(1000 * mult);
            Callback cb = new Callback();
            to.sleepAsync(100 * mult).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    cb.called();
                });
            cb.check();
        }
        {
            //
            // Backward compatible connection timeouts
            //
            TimeoutPrx to = timeout.ice_invocationTimeout(-2).ice_timeout(250);
            com.zeroc.Ice.Connection con = connect(to);
            try
            {
                to.sleep(750);
                test(false);
            }
            catch(com.zeroc.Ice.TimeoutException ex)
            {
                assert(con != null);
                try
                {
                    con.getInfo();
                    test(false);
                }
                catch(com.zeroc.Ice.TimeoutException exc)
                {
                    // Connection got closed as well.
                }
            }
            obj.ice_ping();

            try
            {
                con = connect(to);
                to.sleepAsync(750).join();
                test(false);
            }
            catch(CompletionException ex)
            {
                assert(ex.getCause() instanceof com.zeroc.Ice.TimeoutException);
                assert(con != null);
                try
                {
                    con.getInfo();
                    test(false);
                }
                catch(com.zeroc.Ice.TimeoutException exc)
                {
                    // Connection got closed as well.
                }
            }
            obj.ice_ping();
        }
        out.println("ok");

        out.print("testing close timeout... ");
        out.flush();
        {
            TimeoutPrx to = TimeoutPrx.uncheckedCast(obj.ice_timeout(250 * mult));
            com.zeroc.Ice.Connection connection = connect(to);
            controller.holdAdapter(-1);
            connection.close(com.zeroc.Ice.ConnectionClose.GracefullyWithWait);
            try
            {
                connection.getInfo(); // getInfo() doesn't throw in the closing state.
            }
            catch(com.zeroc.Ice.LocalException ex)
            {
                test(false);
            }
            while(true)
            {
                try
                {
                    connection.getInfo();
                    try
                    {
                        Thread.sleep(10);
                    }
                    catch(java.lang.InterruptedException ex)
                    {
                    }
                }
                catch(com.zeroc.Ice.ConnectionManuallyClosedException ex)
                {
                    // Expected.
                    test(ex.graceful);
                    break;
                }
            }
            controller.resumeAdapter();
            timeout.op(); // Ensure adapter is active.
        }
        out.println("ok");

        out.print("testing timeout overrides... ");
        out.flush();
        {
            //
            // Test Ice.Override.Timeout. This property overrides all
            // endpoint timeouts.
            //
            com.zeroc.Ice.Properties properties = communicator.getProperties()._clone();
            properties.setProperty("Ice.Override.ConnectTimeout", "250");
            properties.setProperty("Ice.Override.Timeout", "100");
            try(com.zeroc.Ice.Communicator comm = helper.initialize(properties))
            {
                TimeoutPrx to = TimeoutPrx.uncheckedCast(comm.stringToProxy(sref));
                connect(to);
                controller.holdAdapter(-1);
                try
                {
                    to.sendData(seq);
                    test(false);
                }
                catch(com.zeroc.Ice.TimeoutException ex)
                {
                    // Expected.
                }
                controller.resumeAdapter();
                timeout.op(); // Ensure adapter is active.

                //
                // Calling ice_timeout() should have no effect.
                //
                to = TimeoutPrx.uncheckedCast(to.ice_timeout(1000 * mult));
                connect(to);
                controller.holdAdapter(-1);
                try
                {
                    to.sendData(seq);
                    test(false);
                }
                catch(com.zeroc.Ice.TimeoutException ex)
                {
                    // Expected.
                }
                controller.resumeAdapter();
                timeout.op(); // Ensure adapter is active.
            }
        }
        {
            //
            // Test Ice.Override.ConnectTimeout.
            //
            com.zeroc.Ice.Properties properties = communicator.getProperties()._clone();
            if(mult == 1)
            {
                properties.setProperty("Ice.Override.ConnectTimeout", "250");
            }
            else
            {
                properties.setProperty("Ice.Override.ConnectTimeout", "2500");
            }

            try(com.zeroc.Ice.Communicator comm = helper.initialize(properties))
            {
                TimeoutPrx to = TimeoutPrx.uncheckedCast(comm.stringToProxy(sref));
                controller.holdAdapter(-1);
                try
                {
                    to.op();
                    test(false);
                }
                catch(com.zeroc.Ice.ConnectTimeoutException ex)
                {
                    // Expected.
                }
                controller.resumeAdapter();
                timeout.op(); // Ensure adapter is active.

                //
                // Calling ice_timeout() should have no effect on the connect timeout.
                //
                controller.holdAdapter(-1);
                to = to.ice_timeout(1000 * mult);
                try
                {
                    to.op();
                    test(false);
                }
                catch(com.zeroc.Ice.ConnectTimeoutException ex)
                {
                    // Expected.
                }
                controller.resumeAdapter();
                timeout.op(); // Ensure adapter is active.

                //
                // Verify that timeout set via ice_timeout() is still used for requests.
                //
                to = to.ice_timeout(250);
                connect(to);
                controller.holdAdapter(-1);
                try
                {
                    to.sendData(seq);
                    test(false);
                }
                catch(com.zeroc.Ice.TimeoutException ex)
                {
                    // Expected.
                }
                controller.resumeAdapter();
                timeout.op(); // Ensure adapter is active.
            }
        }
        {
            //
            // Test Ice.Override.CloseTimeout.
            //
            com.zeroc.Ice.Properties properties = communicator.getProperties()._clone();
            properties.setProperty("Ice.Override.CloseTimeout", "10");
            try(com.zeroc.Ice.Communicator comm = helper.initialize(properties))
            {
                comm.stringToProxy(sref).ice_getConnection();
                controller.holdAdapter(-1);
                long now = System.nanoTime();
                comm.destroy();
                test(System.nanoTime() - now < 2000 * 1000000);
                controller.resumeAdapter();
            }
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
            try
            {
                proxy.sleep(500);
                test(false);
            }
            catch(com.zeroc.Ice.InvocationTimeoutException ex)
            {
            }

            try
            {
                proxy.sleepAsync(500).join();
                test(false);
            }
            catch(CompletionException ex)
            {
                test(ex.getCause() instanceof com.zeroc.Ice.InvocationTimeoutException);
            }

            try
            {
                ((TimeoutPrx)timeout.ice_invocationTimeout(-2)).ice_ping();
                ((TimeoutPrx)timeout.ice_invocationTimeout(-2)).ice_pingAsync().whenComplete((result, ex) ->
                    {
                        test(ex != null);
                    });
            }
            catch(com.zeroc.Ice.Exception ex)
            {
                test(false);
            }

            ((TimeoutPrx)proxy.ice_invocationTimeout(-1)).ice_ping();

            TimeoutPrx batchTimeout = proxy.ice_batchOneway();
            batchTimeout.ice_ping();
            batchTimeout.ice_ping();
            batchTimeout.ice_ping();

            proxy.ice_invocationTimeout(-1).sleepAsync(300); // Keep the server thread pool busy.
            try
            {
                batchTimeout.ice_flushBatchRequests();
                test(false);
            }
            catch(com.zeroc.Ice.InvocationTimeoutException ex)
            {
            }

            batchTimeout.ice_ping();
            batchTimeout.ice_ping();
            batchTimeout.ice_ping();

            proxy.ice_invocationTimeout(-1).sleepAsync(300); // Keep the server thread pool busy.
            try
            {
                batchTimeout.ice_flushBatchRequestsAsync().join();
                test(false);
            }
            catch(CompletionException ex)
            {
                test(ex.getCause() instanceof com.zeroc.Ice.InvocationTimeoutException);
            }

            adapter.destroy();
        }
        out.println("ok");

        controller.shutdown();
    }
}
