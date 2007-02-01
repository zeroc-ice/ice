// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;
using System.Threading;

public class AllTests
{
    private static void test(bool b)
    {
        if(!b)
        {
            throw new Exception();
        }
    }

    private class Callback
    {
        internal Callback()
        {
            _called = false;
        }

        public bool check()
        {
            lock(this)
            {
                while(!_called)
                {
                    Monitor.Wait(this, 5000);

                    if(!_called)
                    {
                        return false; // Must be timeout.
                    }
                }

                _called = false;
                return true;
            }
        }

        public void called()
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

    private class AMISendData : Test.AMI_Timeout_sendData
    {
        public override void ice_response()
        {
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public bool check()
        {
            return callback.check();
        }

        private Callback callback = new Callback();
    }

    private class AMISendDataEx : Test.AMI_Timeout_sendData
    {
        public override void ice_response()
        {
            test(false);
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(ex is Ice.TimeoutException);
            callback.called();
        }

        public bool check()
        {
            return callback.check();
        }

        private Callback callback = new Callback();
    }

    private class AMISleep : Test.AMI_Timeout_sleep
    {
        public override void ice_response()
        {
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public bool check()
        {
            return callback.check();
        }

        private Callback callback = new Callback();
    }

    private class AMISleepEx : Test.AMI_Timeout_sleep
    {
        public override void ice_response()
        {
            test(false);
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(ex is Ice.TimeoutException);
            callback.called();
        }

        public bool check()
        {
            return callback.check();
        }

        private Callback callback = new Callback();
    }

    public static Test.TimeoutPrx allTests(Ice.Communicator communicator)
    {
        string sref = "timeout:default -p 12010 -t 10000";
        Ice.ObjectPrx obj = communicator.stringToProxy(sref);
        test(obj != null);

        Test.TimeoutPrx timeout = Test.TimeoutPrxHelper.checkedCast(obj);
        test(timeout != null);

        Console.Out.Write("testing connect timeout... ");
        Console.Out.Flush();
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
            Test.TimeoutPrx to = Test.TimeoutPrxHelper.uncheckedCast(obj.ice_timeout(1000));
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
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing read timeout... ");
        Console.Out.Flush();
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
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing write timeout... ");
        Console.Out.Flush();
        {
            //
            // Expect TimeoutException.
            //
            Test.TimeoutPrx to = Test.TimeoutPrxHelper.uncheckedCast(obj.ice_timeout(500));
            to.holdAdapter(750);
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
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing AMI read timeout... ");
        Console.Out.Flush();
        {
            //
            // The resolution of AMI timeouts is limited by the connection monitor
            // thread. We set Ice.MonitorConnections=1 (one second) in main().
            //
            // Expect TimeoutException.
            //
            Test.TimeoutPrx to = Test.TimeoutPrxHelper.uncheckedCast(obj.ice_timeout(500));
            AMISleepEx cb = new AMISleepEx();
            to.sleep_async(cb, 2000);
            test(cb.check());
        }
        {
            //
            // Expect success.
            //
            timeout.op(); // Ensure adapter is active.
            Test.TimeoutPrx to = Test.TimeoutPrxHelper.uncheckedCast(obj.ice_timeout(1000));
            AMISleep cb = new AMISleep();
            to.sleep_async(cb, 500);
            test(cb.check());
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing AMI write timeout... ");
        Console.Out.Flush();
        {
            //
            // The resolution of AMI timeouts is limited by the connection monitor
            // thread. We set Ice.MonitorConnections=1 (one second) in main().
            //
            // Expect TimeoutException.
            //
            Test.TimeoutPrx to = Test.TimeoutPrxHelper.uncheckedCast(obj.ice_timeout(500));
            to.holdAdapter(2000);
            byte[] seq = new byte[100000];
            AMISendDataEx cb = new AMISendDataEx();
            to.sendData_async(cb, seq);
            test(cb.check());
        }
        {
            //
            // Expect success.
            //
            timeout.op(); // Ensure adapter is active.
            Test.TimeoutPrx to = Test.TimeoutPrxHelper.uncheckedCast(obj.ice_timeout(1000));
            to.holdAdapter(500);
            byte[] seq = new byte[100000];
            AMISendData cb = new AMISendData();
            to.sendData_async(cb, seq);
            test(cb.check());
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing timeout overrides... ");
        Console.Out.Flush();
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
                to.sleep(1500);
                test(false);
            }
            catch(Ice.TimeoutException)
            {
                // Expected.
            }
            comm.destroy();
        }
        Console.Out.WriteLine("ok");

        return timeout;
    }
}
