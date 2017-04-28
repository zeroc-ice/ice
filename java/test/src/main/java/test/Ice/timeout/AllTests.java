// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.timeout;

import java.io.PrintWriter;
import java.util.concurrent.CompletionException;

import test.Ice.timeout.Test.TimeoutPrx;

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

    public static TimeoutPrx allTests(test.Util.Application app)
    {
        com.zeroc.Ice.Communicator communicator = app.communicator();
        PrintWriter out = app.getWriter();

        String sref = "timeout:" + app.getTestEndpoint(0);
        com.zeroc.Ice.ObjectPrx obj = communicator.stringToProxy(sref);
        test(obj != null);

        int mult = 1;
        if(!communicator.getProperties().getPropertyWithDefault("Ice.Default.Protocol", "tcp").equals("tcp"))
        {
            mult = 4;
        }

        TimeoutPrx timeout = TimeoutPrx.checkedCast(obj);
        test(timeout != null);

        out.print("testing connect timeout... ");
        out.flush();
        {
            //
            // Expect ConnectTimeoutException.
            //
            TimeoutPrx to = timeout.ice_timeout(100 * mult);
            timeout.holdAdapter(500 * mult);
            try
            {
                to.op();
                test(false);
            }
            catch(com.zeroc.Ice.ConnectTimeoutException ex)
            {
                // Expected.
            }
        }
        {
            //
            // Expect success.
            //
            timeout.op(); // Ensure adapter is active.
            TimeoutPrx to = timeout.ice_timeout(1000 * mult);
            timeout.holdAdapter(500 * mult);
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
            TimeoutPrx to = timeout.ice_timeout(100);
            timeout.holdAdapter(500 * mult);
            try
            {
                to.sendData(seq);
                test(false);
            }
            catch(com.zeroc.Ice.TimeoutException ex)
            {
                // Expected.
            }
        }
        {
            //
            // Expect success.
            //
            timeout.op(); // Ensure adapter is active.
            TimeoutPrx to = timeout.ice_timeout(1000 * mult);
            timeout.holdAdapter(500 * mult);
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
                to.sleep(750 * mult);
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
            to.sleepAsync(750 * mult).whenComplete((result, ex) ->
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
            TimeoutPrx to = timeout.ice_invocationTimeout(500 * mult);
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
            com.zeroc.Ice.Connection con = to.ice_getConnection();
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
                con = to.ice_getConnection();
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
            TimeoutPrx to = TimeoutPrx.checkedCast(obj.ice_timeout(250 * mult));
            com.zeroc.Ice.Connection connection = to.ice_getConnection();
            timeout.holdAdapter(600);
            connection.close(com.zeroc.Ice.ConnectionClose.GracefullyWithWait);
            try
            {
                connection.getInfo(); // getInfo() doesn't throw in the closing state.
            }
            catch(com.zeroc.Ice.LocalException ex)
            {
                test(false);
            }
            try
            {
                Thread.sleep(650 * mult);
            }
            catch(java.lang.InterruptedException ex)
            {
            }
            try
            {
                connection.getInfo();
                test(false);
            }
            catch(com.zeroc.Ice.ConnectionManuallyClosedException ex)
            {
                // Expected.
                test(ex.graceful);
            }
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
            com.zeroc.Ice.InitializationData initData = app.createInitializationData();
            initData.properties = communicator.getProperties()._clone();
            initData.properties.setProperty("Ice.Override.Timeout", "250");
            com.zeroc.Ice.Communicator comm = app.initialize(initData);
            TimeoutPrx to = TimeoutPrx.checkedCast(comm.stringToProxy(sref));
            timeout.holdAdapter(700 * mult);
            try
            {
                to.sendData(seq);
                test(false);
            }
            catch(com.zeroc.Ice.TimeoutException ex)
            {
                // Expected.
            }
            //
            // Calling ice_timeout() should have no effect.
            //
            timeout.op(); // Ensure adapter is active.
            to = TimeoutPrx.checkedCast(to.ice_timeout(1000 * mult));
            timeout.holdAdapter(500 * mult);
            try
            {
                to.sendData(seq);
                test(false);
            }
            catch(com.zeroc.Ice.TimeoutException ex)
            {
                // Expected.
            }
            comm.destroy();
        }
        {
            //
            // Test Ice.Override.ConnectTimeout.
            //
            com.zeroc.Ice.InitializationData initData = app.createInitializationData();
            initData.properties = communicator.getProperties()._clone();
            if(mult == 1)
            {
                initData.properties.setProperty("Ice.Override.ConnectTimeout", "250");
            }
            else
            {
                initData.properties.setProperty("Ice.Override.ConnectTimeout", "2500");
            }

            com.zeroc.Ice.Communicator comm = app.initialize(initData);
            TimeoutPrx to = TimeoutPrx.uncheckedCast(comm.stringToProxy(sref));
            timeout.holdAdapter(750 * mult);
            try
            {
                to.op();
                test(false);
            }
            catch(com.zeroc.Ice.ConnectTimeoutException ex)
            {
                // Expected.
            }
            //
            // Calling ice_timeout() should have no effect on the connect timeout.
            //
            timeout.op(); // Ensure adapter is active.
            timeout.holdAdapter(750 * mult);
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
            //
            // Verify that timeout set via ice_timeout() is still used for requests.
            //
            timeout.op(); // Ensure adapter is active.
            to = to.ice_timeout(250);
            to.ice_getConnection(); // Establish connection
            timeout.holdAdapter(750 * mult);
            try
            {
                to.sendData(seq);
                test(false);
            }
            catch(com.zeroc.Ice.TimeoutException ex)
            {
                // Expected.
            }
            comm.destroy();
        }
        {
            //
            // Test Ice.Override.CloseTimeout.
            //
            com.zeroc.Ice.InitializationData initData = app.createInitializationData();
            initData.properties = communicator.getProperties()._clone();
            initData.properties.setProperty("Ice.Override.CloseTimeout", "100");
            com.zeroc.Ice.Communicator comm = app.initialize(initData);
            comm.stringToProxy(sref).ice_getConnection();
            timeout.holdAdapter(800);
            long now = System.nanoTime();
            comm.destroy();
            test(System.nanoTime() - now < 700 * 1000000);
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

        return timeout;
    }
}
