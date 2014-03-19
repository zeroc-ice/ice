// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;
using System.Threading;

#if SILVERLIGHT
using System.Windows.Controls;
#endif

public class AllTests : TestCommon.TestApp
{
    private class CallbackBase
    {
        internal CallbackBase()
        {
            _called = false;
        }

        public virtual void check()
        {
            _m.Lock();
            try
            {
                while(!_called)
                {
                    _m.Wait();
                }

                _called = false;
            }
            finally
            {
                _m.Unlock();
            }
        }

        public virtual void called()
        {
            _m.Lock();
            try
            {
                Debug.Assert(!_called);
                _called = true;
                _m.Notify();
            }
            finally
            {
                _m.Unlock();
            }
        }

        private bool _called;
        private readonly IceUtilInternal.Monitor _m = new IceUtilInternal.Monitor();
    }

    private class Callback
    {
        public void response()
        {
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public void responseEx()
        {
            test(false);
        }

        public void exceptionEx(Ice.Exception ex)
        {
            test(ex is Ice.TimeoutException);
            callback.called();
        }

        public void check()
        {
            callback.check();
        }

        private CallbackBase callback = new CallbackBase();
    }

#if SILVERLIGHT
    public override Ice.InitializationData initData()
    {
        Ice.InitializationData initData = new Ice.InitializationData();
        initData.properties = Ice.Util.createProperties();
        //
        // For this test, we want to disable retries.
        //
        initData.properties.setProperty("Ice.RetryIntervals", "-1");

        //
        // This test kills connections, so we don't want warnings.
        //
        initData.properties.setProperty("Ice.Warn.Connections", "0");
        return initData;
    }

    override
    public void run(Ice.Communicator communicator)
#else
    public static Test.TimeoutPrx allTests(Ice.Communicator communicator)
#endif
    {
        string sref = "timeout:default -p 12010";
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
            Test.TimeoutPrx to = Test.TimeoutPrxHelper.uncheckedCast(obj.ice_timeout(500));
            to.holdAdapter(750);
            to.ice_getConnection().close(true); // Force a reconnect.
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
            Test.TimeoutPrx to = Test.TimeoutPrxHelper.uncheckedCast(obj.ice_timeout(2000));
            to.holdAdapter(500);
            to.ice_getConnection().close(true); // Force a reconnect.
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

        Write("testing read timeout... ");
        Flush();
        {
            //
            // Expect TimeoutException.
            //
            Test.TimeoutPrx to = Test.TimeoutPrxHelper.uncheckedCast(obj.ice_timeout(500));
            try
            {
                to.sleep(750);
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
            try
            {
                to.sleep(500);
            }
            catch(Ice.TimeoutException)
            {
                test(false);
            }
        }
        WriteLine("ok");

        Write("testing write timeout... ");
        Flush();
        {
            //
            // Expect TimeoutException.
            //
            Test.TimeoutPrx to = Test.TimeoutPrxHelper.uncheckedCast(obj.ice_timeout(500));
            to.holdAdapter(2000);
            try
            {
                byte[] seq = new byte[100000];
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
            to.holdAdapter(500);
            try
            {
                byte[] seq = new byte[100000];
                to.sendData(seq);
            }
            catch(Ice.TimeoutException)
            {
                test(false);
            }
        }
        WriteLine("ok");

        Write("testing AMI read timeout... ");
        Flush();
        {
            //
            // Expect TimeoutException.
            //
            Test.TimeoutPrx to = Test.TimeoutPrxHelper.uncheckedCast(obj.ice_timeout(500));
            Callback cb = new Callback();
            to.begin_sleep(2000).whenCompleted(cb.responseEx, cb.exceptionEx);
            cb.check();
        }
        {
            //
            // Expect success.
            //
            timeout.op(); // Ensure adapter is active.
            Test.TimeoutPrx to = Test.TimeoutPrxHelper.uncheckedCast(obj.ice_timeout(1000));
            Callback cb = new Callback();
            to.begin_sleep(500).whenCompleted(cb.response, cb.exception);
            cb.check();
        }
        WriteLine("ok");

        Write("testing AMI write timeout... ");
        Flush();
        {
            //
            // Expect TimeoutException.
            //
            Test.TimeoutPrx to = Test.TimeoutPrxHelper.uncheckedCast(obj.ice_timeout(500));
            to.holdAdapter(2000);
            byte[] seq = new byte[100000];
            Callback cb = new Callback();
            to.begin_sendData(seq).whenCompleted(cb.responseEx, cb.exceptionEx);
            cb.check();
        }
        {
            //
            // Expect success.
            //
            timeout.op(); // Ensure adapter is active.
            Test.TimeoutPrx to = Test.TimeoutPrxHelper.uncheckedCast(obj.ice_timeout(1000));
            to.holdAdapter(500);
            byte[] seq = new byte[100000];
            Callback cb = new Callback();
            to.begin_sendData(seq).whenCompleted(cb.response, cb.exception);
            cb.check();
        }
        WriteLine("ok");

        Write("testing close timeout... ");
        Flush();
        {
            Test.TimeoutPrx to = Test.TimeoutPrxHelper.checkedCast(obj.ice_timeout(250));
            Ice.Connection connection = to.ice_getConnection();
            timeout.holdAdapter(750);
            connection.close(false);
            try
            {
                connection.getInfo(); // getInfo() doesn't throw in the closing state.
            }
            catch(Ice.LocalException)
            {
                test(false);
            }
            Thread.Sleep(500);
            try
            {
                connection.getInfo();
                test(false);
            }
            catch(Ice.CloseConnectionException)
            {
                // Expected.
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
            initData.properties.setProperty("Ice.Override.Timeout", "500");
            Ice.Communicator comm = Ice.Util.initialize(ref args, initData);
            Test.TimeoutPrx to = Test.TimeoutPrxHelper.checkedCast(comm.stringToProxy(sref));
            try
            {
                to.sleep(750);
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
            try
            {
                to.sleep(750);
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
            initData.properties.setProperty("Ice.Override.ConnectTimeout", "750");
            Ice.Communicator comm = Ice.Util.initialize(ref args, initData);
            timeout.holdAdapter(1000);
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
            timeout.holdAdapter(1000);
            to = Test.TimeoutPrxHelper.uncheckedCast(to.ice_timeout(1250));
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
            to.op(); // Force connection.
            try
            {
                to.sleep(2000);
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
            initData.properties.setProperty("Ice.Override.CloseTimeout", "200");
            Ice.Communicator comm = Ice.Util.initialize(initData);
            comm.stringToProxy(sref).ice_getConnection();
            timeout.holdAdapter(750);
            timeout.holdAdapter(750);
            long begin = System.DateTime.Now.Ticks;
            comm.destroy();
            test(((long)new System.TimeSpan(System.DateTime.Now.Ticks - begin).TotalMilliseconds - begin) < 500);
        }
        WriteLine("ok");
#if SILVERLIGHT
        timeout.shutdown();
#else
        return timeout;
#endif
    }
}
