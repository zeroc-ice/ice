// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Diagnostics;
using System.Threading;

public class AllTests : TestCommon.AllTests
{
    private class Callback
    {
        internal Callback()
        {
            _called = false;
        }

        public virtual void check()
        {
            lock(this)
            {
                while(!_called)
                {
                    Monitor.Wait(this);
                }

                _called = false;
            }
        }

        public virtual void called()
        {
            lock(this)
            {
                Debug.Assert(!_called);
                _called = true;
                Monitor.Pulse(this);
            }
        }

        private bool _called;
    }

    public static Test.TimeoutPrx allTests(TestCommon.Application app)
    {
        Ice.Communicator communicator = app.communicator();
        string sref = "timeout:" + app.getTestEndpoint(0);
        Ice.ObjectPrx obj = communicator.stringToProxy(sref);
        test(obj != null);

        Test.TimeoutPrx timeout = Test.TimeoutPrxHelper.checkedCast(obj);
        test(timeout != null);

        Write("testing connect timeout... ");
        Flush();
        {
            //
            // Expect ConnectTimeoutException.
            //
            Test.TimeoutPrx to = Test.TimeoutPrxHelper.uncheckedCast(obj.ice_timeout(100));
            timeout.holdAdapter(500);
            try
            {
                to.op();
                test(false);
            }
            catch(Ice.ConnectTimeoutException)
            {
                // Expected.
            }
        }
        {
            //
            // Expect success.
            //
            timeout.op(); // Ensure adapter is active.
            Test.TimeoutPrx to = Test.TimeoutPrxHelper.uncheckedCast(obj.ice_timeout(1000));
            timeout.holdAdapter(500);
            try
            {
                to.op();
            }
            catch(Ice.ConnectTimeoutException)
            {
                test(false);
            }
        }
        WriteLine("ok");

        // The sequence needs to be large enough to fill the write/recv buffers
        byte[] seq = new byte[2000000];

        Write("testing connection timeout... ");
        Flush();
        {
            //
            // Expect TimeoutException.
            //
            Test.TimeoutPrx to = Test.TimeoutPrxHelper.uncheckedCast(obj.ice_timeout(100));
            timeout.holdAdapter(500);
            try
            {
                to.sendData(seq);
                test(false);
            }
            catch(Ice.TimeoutException)
            {
                // Expected.
            }
        }
        {
            //
            // Expect success.
            //
            timeout.op(); // Ensure adapter is active.
            Test.TimeoutPrx to = Test.TimeoutPrxHelper.uncheckedCast(obj.ice_timeout(1000));
            timeout.holdAdapter(500);
            try
            {
                to.sendData(new byte[1000000]);
            }
            catch(Ice.TimeoutException)
            {
                test(false);
            }
        }
        WriteLine("ok");

        Write("testing invocation timeout... ");
        Flush();
        {
            Ice.Connection connection = obj.ice_getConnection();
            Test.TimeoutPrx to = Test.TimeoutPrxHelper.uncheckedCast(obj.ice_invocationTimeout(100));
            test(connection == to.ice_getConnection());
            try
            {
                to.sleep(750);
                test(false);
            }
            catch(Ice.InvocationTimeoutException)
            {
            }
            obj.ice_ping();
            to = Test.TimeoutPrxHelper.checkedCast(obj.ice_invocationTimeout(500));
            test(connection == to.ice_getConnection());
            try
            {
                to.sleep(100);
            }
            catch(Ice.InvocationTimeoutException)
            {
                test(false);
            }
            test(connection == to.ice_getConnection());
        }
        {
            //
            // Expect InvocationTimeoutException.
            //
            Test.TimeoutPrx to = Test.TimeoutPrxHelper.uncheckedCast(obj.ice_invocationTimeout(100));
            Callback cb = new Callback();
            to.begin_sleep(750).whenCompleted(
                () =>
                {
                    test(false);
                },
                (Ice.Exception ex) =>
                {
                    test(ex is Ice.InvocationTimeoutException);
                    cb.called();
                });
            cb.check();
            obj.ice_ping();
        }
        {
            //
            // Expect success.
            //
            Test.TimeoutPrx to = Test.TimeoutPrxHelper.uncheckedCast(obj.ice_invocationTimeout(500));
            Callback cb = new Callback();
            to.begin_sleep(100).whenCompleted(
                () =>
                {
                    cb.called();

                },
                (Ice.Exception ex) =>
                {
                    test(false);
                });
            cb.check();
        }
        {
            //
            // Backward compatible connection timeouts
            //
            Test.TimeoutPrx to = Test.TimeoutPrxHelper.uncheckedCast(obj.ice_invocationTimeout(-2).ice_timeout(250));
            Ice.Connection con = to.ice_getConnection();
            try
            {
                to.sleep(750);
                test(false);
            }
            catch(Ice.TimeoutException)
            {
                try
                {
                    con.getInfo();
                    test(false);
                }
                catch(Ice.TimeoutException)
                {
                    // Connection got closed as well.
                }
            }
            obj.ice_ping();

            try
            {
                con = to.ice_getConnection();
                to.end_sleep(to.begin_sleep(750));
                test(false);
            }
            catch(Ice.TimeoutException)
            {
                try
                {
                    con.getInfo();
                    test(false);
                }
                catch(Ice.TimeoutException)
                {
                    // Connection got closed as well.
                }
            }
            obj.ice_ping();
        }
        WriteLine("ok");

        Write("testing close timeout... ");
        Flush();
        {
            Test.TimeoutPrx to = Test.TimeoutPrxHelper.checkedCast(obj.ice_timeout(250));
            int nRetry = 5;
            while(--nRetry > 0)
            {
                try
                {
                    to.ice_getConnection();
                    break;
                }
                catch(Ice.ConnectTimeoutException)
                {
                    // Can sporadically occur with slow machines
                }
            }
            Ice.Connection connection = to.ice_getConnection();
            timeout.holdAdapter(600);
            connection.close(Ice.ConnectionClose.GracefullyWithWait);
            try
            {
                connection.getInfo(); // getInfo() doesn't throw in the closing state.
            }
            catch(Ice.LocalException)
            {
                test(false);
            }
            Thread.Sleep(650);
            try
            {
                connection.getInfo();
                test(false);
            }
            catch(Ice.ConnectionManuallyClosedException ex)
            {
                // Expected.
                test(ex.graceful);
            }
            timeout.op(); // Ensure adapter is active.
        }
        WriteLine("ok");

        Write("testing timeout overrides... ");
        Flush();
        {
            //
            // Test Ice.Override.Timeout. This property overrides all
            // endpoint timeouts.
            //
            string[] args = new string[0];
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = communicator.getProperties().ice_clone_();
            initData.properties.setProperty("Ice.Override.Timeout", "250");
            Ice.Communicator comm = Ice.Util.initialize(ref args, initData);
            Test.TimeoutPrx to = Test.TimeoutPrxHelper.checkedCast(comm.stringToProxy(sref));
            timeout.holdAdapter(700);
            try
            {
                to.sendData(seq);
                test(false);
            }
            catch(Ice.TimeoutException)
            {
                // Expected.
            }
            //
            // Calling ice_timeout() should have no effect.
            //
            timeout.op(); // Ensure adapter is active.
            to = Test.TimeoutPrxHelper.checkedCast(to.ice_timeout(1000));
            timeout.holdAdapter(500);
            try
            {
                to.sendData(seq);
                test(false);
            }
            catch(Ice.TimeoutException)
            {
                // Expected.
            }
            comm.destroy();
        }
        {
            //
            // Test Ice.Override.ConnectTimeout.
            //
            string[] args = new string[0];
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = communicator.getProperties().ice_clone_();
            initData.properties.setProperty("Ice.Override.ConnectTimeout", "250");
            Ice.Communicator comm = Ice.Util.initialize(ref args, initData);
            timeout.holdAdapter(750);
            Test.TimeoutPrx to = Test.TimeoutPrxHelper.uncheckedCast(comm.stringToProxy(sref));
            try
            {
                to.op();
                test(false);
            }
            catch(Ice.ConnectTimeoutException)
            {
                // Expected.
            }
            //
            // Calling ice_timeout() should have no effect on the connect timeout.
            //
            timeout.op(); // Ensure adapter is active.
            timeout.holdAdapter(750);
            to = Test.TimeoutPrxHelper.uncheckedCast(to.ice_timeout(1000));
            try
            {
                to.op();
                test(false);
            }
            catch(Ice.ConnectTimeoutException)
            {
                // Expected.
            }
            //
            // Verify that timeout set via ice_timeout() is still used for requests.
            //
            timeout.op(); // Ensure adapter is active.
            to = Test.TimeoutPrxHelper.uncheckedCast(to.ice_timeout(250));
            int nRetry = 5;
            while(--nRetry > 0)
            {
                try
                {
                    to.ice_getConnection();
                    break;
                }
                catch(Ice.ConnectTimeoutException)
                {
                    // Can sporadically occur with slow machines
                }
            }
            to.ice_getConnection(); // Establish connection.
            timeout.holdAdapter(750);
            try
            {
                to.sendData(seq);
                test(false);
            }
            catch(Ice.TimeoutException)
            {
                // Expected.
            }
            comm.destroy();
        }
        {
            //
            // Test Ice.Override.CloseTimeout.
            //
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = communicator.getProperties().ice_clone_();
            initData.properties.setProperty("Ice.Override.CloseTimeout", "100");
            Ice.Communicator comm = Ice.Util.initialize(initData);
            comm.stringToProxy(sref).ice_getConnection();
            timeout.holdAdapter(800);
            long begin = System.DateTime.Now.Ticks;
            comm.destroy();
            test(((long)new System.TimeSpan(System.DateTime.Now.Ticks - begin).TotalMilliseconds - begin) < 700);
        }
        WriteLine("ok");

        Write("testing invocation timeouts with collocated calls... ");
        Flush();
        {
            communicator.getProperties().setProperty("TimeoutCollocated.AdapterId", "timeoutAdapter");

            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TimeoutCollocated");
            adapter.activate();

            Test.TimeoutPrx proxy = Test.TimeoutPrxHelper.uncheckedCast(adapter.addWithUUID(new TimeoutI()));
            proxy = (Test.TimeoutPrx)proxy.ice_invocationTimeout(100);
            try
            {
                proxy.sleep(500);
                test(false);
            }
            catch(Ice.InvocationTimeoutException)
            {
            }

            try
            {
                proxy.end_sleep(proxy.begin_sleep(500));
                test(false);
            }
            catch(Ice.InvocationTimeoutException)
            {
            }

            try
            {
                ((Test.TimeoutPrx)proxy.ice_invocationTimeout(-2)).ice_ping();
                ((Test.TimeoutPrx)proxy.ice_invocationTimeout(-2)).begin_ice_ping().waitForCompleted();
            }
            catch(Ice.Exception)
            {
                test(false);
            }

            Test.TimeoutPrx batchTimeout = (Test.TimeoutPrx)proxy.ice_batchOneway();
            batchTimeout.ice_ping();
            batchTimeout.ice_ping();
            batchTimeout.ice_ping();

            ((Test.TimeoutPrx)proxy.ice_invocationTimeout(-1)).begin_sleep(300); // Keep the server thread pool busy.
            try
            {
                batchTimeout.ice_flushBatchRequests();
                test(false);
            }
            catch(Ice.InvocationTimeoutException)
            {
            }

            batchTimeout.ice_ping();
            batchTimeout.ice_ping();
            batchTimeout.ice_ping();

            ((Test.TimeoutPrx)proxy.ice_invocationTimeout(-1)).begin_sleep(300); // Keep the server thread pool busy.
            try
            {
                batchTimeout.end_ice_flushBatchRequests(batchTimeout.begin_ice_flushBatchRequests());
                test(false);
            }
            catch(Ice.InvocationTimeoutException)
            {
            }


            adapter.destroy();
        }
        WriteLine("ok");
        return timeout;
    }
}
