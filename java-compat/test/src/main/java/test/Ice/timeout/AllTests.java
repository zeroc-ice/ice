// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.timeout;

import java.io.PrintWriter;

import test.Ice.timeout.Test.TimeoutPrx;
import test.Ice.timeout.Test.TimeoutPrxHelper;
import test.Ice.timeout.Test.ControllerPrx;
import test.Ice.timeout.Test.ControllerPrxHelper;

public class AllTests
{
    private static void
    test(boolean b)
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

        public synchronized void
        check()
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

        public synchronized void
        called()
        {
            assert(!_called);
            _called = true;
            notify();
        }

        private boolean _called;
    }

    private static class CallbackSuccess extends Ice.Callback
    {
        @Override
        public void
        completed(Ice.AsyncResult result)
        {
            try
            {
                TimeoutPrx p = TimeoutPrxHelper.uncheckedCast(result.getProxy());
                if(result.getOperation().equals("sendData"))
                {
                    p.end_sendData(result);
                }
                else if(result.getOperation().equals("sleep"))
                {
                    p.end_sleep(result);
                }
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
            callback.called();
        }

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class CallbackFail extends Ice.Callback
    {
        @Override
        public void
        completed(Ice.AsyncResult result)
        {
            try
            {
                TimeoutPrx p = TimeoutPrxHelper.uncheckedCast(result.getProxy());
                if(result.getOperation().equals("sendData"))
                {
                    p.end_sendData(result);
                }
                else if(result.getOperation().equals("sleep"))
                {
                    p.end_sleep(result);
                }
                test(false);
            }
            catch(Ice.TimeoutException ex)
            {
                callback.called();
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
        }

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    public static Ice.Connection
    connect(Ice.ObjectPrx prx)
    {
        int nRetry = 10;
        while(--nRetry > 0)
        {
            try
            {
                prx.ice_getConnection();
                break;
            }
            catch(Ice.ConnectTimeoutException ex)
            {
                // Can sporadically occur with slow machines
            }
        }
        return prx.ice_getConnection(); // Establish connection
    }

    public static void
    allTests(test.TestHelper helper)
    {
        Ice.Communicator communicator = helper.communicator();
        PrintWriter out = helper.getWriter();

        String sref = "timeout:" + helper.getTestEndpoint(0);
        Ice.ObjectPrx obj = communicator.stringToProxy(sref);
        test(obj != null);

        int mult = 1;
        if(!communicator.getProperties().getPropertyWithDefault("Ice.Default.Protocol", "tcp").equals("tcp") ||
           helper.isAndroid())
        {
            mult = 4;
        }

        TimeoutPrx timeout = TimeoutPrxHelper.checkedCast(obj);
        test(timeout != null);

        ControllerPrx controller = ControllerPrxHelper.checkedCast(
           communicator.stringToProxy("controller:" + helper.getTestEndpoint(1)));
        test(controller != null);

        out.print("testing connect timeout... ");
        out.flush();
        {
            //
            // Expect ConnectTimeoutException.
            //
            TimeoutPrx to = TimeoutPrxHelper.uncheckedCast(obj.ice_timeout(100 * mult));
            controller.holdAdapter(-1);
            try
            {
                to.op();
                test(false);
            }
            catch(Ice.ConnectTimeoutException ex)
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
            timeout.op(); // Ensure adapter is active.
            TimeoutPrx to = TimeoutPrxHelper.uncheckedCast(obj.ice_timeout(2000 * mult));
            controller.holdAdapter(100 * mult);
            try
            {
                to.op();
            }
            catch(Ice.ConnectTimeoutException ex)
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
            TimeoutPrx to = TimeoutPrxHelper.uncheckedCast(obj.ice_timeout(250));
            connect(to);
            controller.holdAdapter(-1);
            try
            {
                to.sendData(seq);
                test(false);
            }
            catch(Ice.TimeoutException ex)
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
            TimeoutPrx to = TimeoutPrxHelper.uncheckedCast(obj.ice_timeout(1000 * mult));
            controller.holdAdapter(100 * mult);
            try
            {
                to.sendData(new byte[1000000]);
            }
            catch(Ice.TimeoutException ex)
            {
                test(false);
            }
        }
        out.println("ok");

        out.print("testing invocation timeout... ");
        out.flush();
        {
            Ice.Connection connection = obj.ice_getConnection();
            TimeoutPrx to = TimeoutPrxHelper.uncheckedCast(obj.ice_invocationTimeout(100));
            test(connection == to.ice_getConnection());
            try
            {
                to.sleep(500 * mult);
                test(false);
            }
            catch(Ice.InvocationTimeoutException ex)
            {
            }
            obj.ice_ping();
            to = TimeoutPrxHelper.checkedCast(obj.ice_invocationTimeout(500 * mult));
            test(connection == to.ice_getConnection());
            try
            {
                to.sleep(100 * mult);
            }
            catch(Ice.InvocationTimeoutException ex)
            {
                test(false);
            }
            test(connection == to.ice_getConnection());
        }
        {
            //
            // Expect InvocationTimeoutException.
            //
            TimeoutPrx to = TimeoutPrxHelper.uncheckedCast(obj.ice_invocationTimeout(100));
            CallbackFail cb = new CallbackFail();
            to.begin_sleep(500 * mult, cb);
            cb.check();
            obj.ice_ping();
        }
        {
            //
            // Expect success.
            //
            TimeoutPrx to = TimeoutPrxHelper.uncheckedCast(obj.ice_invocationTimeout(1000 * mult));
            CallbackSuccess cb = new CallbackSuccess();
            to.begin_sleep(100 * mult, cb);
            cb.check();
        }
        {
            //
            // Backward compatible connection timeouts
            //
            TimeoutPrx to = TimeoutPrxHelper.uncheckedCast(obj.ice_invocationTimeout(-2).ice_timeout(250));
            Ice.Connection con = connect(to);
            try
            {
                to.sleep(750);
                test(false);
            }
            catch(Ice.TimeoutException ex)
            {
                assert(con != null);
                try
                {
                    con.getInfo();
                    test(false);
                }
                catch(Ice.TimeoutException exc)
                {
                    // Connection got closed as well.
                }
            }
            obj.ice_ping();

            try
            {
                con = connect(to);
                to.end_sleep(to.begin_sleep(750));
                test(false);
            }
            catch(Ice.TimeoutException ex)
            {
                assert(con != null);
                try
                {
                    con.getInfo();
                    test(false);
                }
                catch(Ice.TimeoutException exc)
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
            TimeoutPrx to = TimeoutPrxHelper.uncheckedCast(obj.ice_timeout(250 * mult));
            Ice.Connection connection = connect(to);
            controller.holdAdapter(-1);
            connection.close(Ice.ConnectionClose.GracefullyWithWait);
            try
            {
                connection.getInfo(); // getInfo() doesn't throw in the closing state.
            }
            catch(Ice.LocalException ex)
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
                catch(Ice.ConnectionManuallyClosedException ex)
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
            Ice.Properties properties = communicator.getProperties()._clone();
            properties.setProperty("Ice.Override.ConnectTimeout", "250");
            properties.setProperty("Ice.Override.Timeout", "100");

            try(Ice.Communicator comm = helper.initialize(properties))
            {
                TimeoutPrx to = TimeoutPrxHelper.uncheckedCast(comm.stringToProxy(sref));
                connect(to);
                controller.holdAdapter(-1);
                try
                {
                    to.sendData(seq);
                    test(false);
                }
                catch(Ice.TimeoutException ex)
                {
                    // Expected.
                }
                controller.resumeAdapter();
                timeout.op(); // Ensure adapter is active.

                //
                // Calling ice_timeout() should have no effect.
                //
                to = TimeoutPrxHelper.uncheckedCast(to.ice_timeout(1000 * mult));
                connect(to);
                controller.holdAdapter(-1);
                try
                {
                    to.sendData(seq);
                    test(false);
                }
                catch(Ice.TimeoutException ex)
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
            Ice.Properties properties = communicator.getProperties()._clone();
            if(mult == 1)
            {
                properties.setProperty("Ice.Override.ConnectTimeout", "250");
            }
            else
            {
                properties.setProperty("Ice.Override.ConnectTimeout", "2500");
            }

            try(Ice.Communicator comm = helper.initialize(properties))
            {
                TimeoutPrx to = TimeoutPrxHelper.uncheckedCast(comm.stringToProxy(sref));
                controller.holdAdapter(-1);
                try
                {
                    to.op();
                    test(false);
                }
                catch(Ice.ConnectTimeoutException ex)
                {
                    // Expected.
                }
                controller.resumeAdapter();
                timeout.op(); // Ensure adapter is active.

                //
                // Calling ice_timeout() should have no effect on the connect timeout.
                //
                controller.holdAdapter(-1);
                to = TimeoutPrxHelper.uncheckedCast(to.ice_timeout(1000 * mult));
                try
                {
                    to.op();
                    test(false);
                }
                catch(Ice.ConnectTimeoutException ex)
                {
                    // Expected.
                }
                controller.resumeAdapter();
                timeout.op(); // Ensure adapter is active.

                //
                // Verify that timeout set via ice_timeout() is still used for requests.
                //
                to = TimeoutPrxHelper.uncheckedCast(to.ice_timeout(250));
                connect(to);
                controller.holdAdapter(-1);
                try
                {
                    to.sendData(seq);
                    test(false);
                }
                catch(Ice.TimeoutException ex)
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
            Ice.Properties properties = communicator.getProperties()._clone();
            properties.setProperty("Ice.Override.CloseTimeout", "10");
            Ice.Communicator comm = helper.initialize(properties);
            comm.stringToProxy(sref).ice_getConnection();
            controller.holdAdapter(-1);
            long now = System.nanoTime();
            comm.destroy();
            test(System.nanoTime() - now < 2000 * 1000000);
            controller.resumeAdapter();
        }
        out.println("ok");

        out.print("testing invocation timeouts with collocated calls... ");
        out.flush();
        {
            communicator.getProperties().setProperty("TimeoutCollocated.AdapterId", "timeoutAdapter");

            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TimeoutCollocated");
            adapter.activate();

            TimeoutPrx proxy = TimeoutPrxHelper.uncheckedCast(adapter.addWithUUID(new TimeoutI()));
            proxy = (TimeoutPrx)proxy.ice_invocationTimeout(100);
            try
            {
                proxy.sleep(500);
                test(false);
            }
            catch(Ice.InvocationTimeoutException ex)
            {
            }

            try
            {
                proxy.end_sleep(proxy.begin_sleep(500));
                test(false);
            }
            catch(Ice.InvocationTimeoutException ex)
            {
            }

            try
            {
                ((TimeoutPrx)timeout.ice_invocationTimeout(-2)).ice_ping();
                ((TimeoutPrx)timeout.ice_invocationTimeout(-2)).begin_ice_ping().waitForCompleted();
            }
            catch(Ice.Exception ex)
            {
                test(false);
            }

            ((TimeoutPrx)proxy.ice_invocationTimeout(-1)).ice_ping();

            TimeoutPrx batchTimeout = (TimeoutPrx)proxy.ice_batchOneway();
            batchTimeout.ice_ping();
            batchTimeout.ice_ping();
            batchTimeout.ice_ping();

            ((TimeoutPrx)proxy.ice_invocationTimeout(-1)).begin_sleep(300); // Keep the server thread pool busy.
            try
            {
                batchTimeout.ice_flushBatchRequests();
                test(false);
            }
            catch(Ice.InvocationTimeoutException ex)
            {
            }

            batchTimeout.ice_ping();
            batchTimeout.ice_ping();
            batchTimeout.ice_ping();

            ((TimeoutPrx)proxy.ice_invocationTimeout(-1)).begin_sleep(300); // Keep the server thread pool busy.
            try
            {
                batchTimeout.end_ice_flushBatchRequests(batchTimeout.begin_ice_flushBatchRequests());
                test(false);
            }
            catch(Ice.InvocationTimeoutException ex)
            {
            }

            adapter.destroy();
        }
        out.println("ok");

        controller.shutdown();
    }
}
