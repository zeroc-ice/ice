// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Test.*;

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

        public synchronized boolean
        check()
        {
            while(!_called)
            {
                try
                {
                    wait(5000);
                }
                catch(InterruptedException ex)
                {
                    continue;
                }

                if(!_called)
                {
                    return false; // Must be timeout.
                }
            }

            _called = false;
            return true;
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

    private static class AMISendData extends Test.AMI_Timeout_sendData
    {
        public void
        ice_response()
        {
            callback.called();
        }

        public void
        ice_exception(Ice.LocalException ex)
        {
            test(false);
        }

        public boolean
        check()
        {
            return callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMISendDataEx extends Test.AMI_Timeout_sendData
    {
        public void
        ice_response()
        {
            test(false);
        }

        public void
        ice_exception(Ice.LocalException ex)
        {
            test(ex instanceof Ice.TimeoutException);
            callback.called();
        }

        public boolean
        check()
        {
            return callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMISleep extends Test.AMI_Timeout_sleep
    {
        public void
        ice_response()
        {
            callback.called();
        }

        public void
        ice_exception(Ice.LocalException ex)
        {
            test(false);
        }

        public boolean
        check()
        {
            return callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMISleepEx extends Test.AMI_Timeout_sleep
    {
        public void
        ice_response()
        {
            test(false);
        }

        public void
        ice_exception(Ice.LocalException ex)
        {
            test(ex instanceof Ice.TimeoutException);
            callback.called();
        }

        public boolean
        check()
        {
            return callback.check();
        }

        private Callback callback = new Callback();
    }

    public static Test.TimeoutPrx
    allTests(Ice.Communicator communicator, java.io.PrintStream out)
    {
        String sref = "timeout:default -p 12010 -t 10000";
        Ice.ObjectPrx obj = communicator.stringToProxy(sref);
        test(obj != null);

        TimeoutPrx timeout = TimeoutPrxHelper.checkedCast(obj);
        test(timeout != null);

        out.print("testing connect timeout... ");
        out.flush();
        {
            //
            // Expect ConnectTimeoutException.
            //
            TimeoutPrx to = TimeoutPrxHelper.uncheckedCast(obj.ice_timeout(500));
            to.holdAdapter(2000);
            to.ice_getConnection().close(true); // Force a reconnect.
            try
            {
                to.op();
                test(false);
            }
            catch(Ice.ConnectTimeoutException ex)
            {
                // Expected.
            }
        }
        {
            //
            // Expect success.
            //
            timeout.op(); // Ensure adapter is active.
            TimeoutPrx to = TimeoutPrxHelper.uncheckedCast(obj.ice_timeout(1000));
            to.holdAdapter(500);
            to.ice_getConnection().close(true); // Force a reconnect.
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

        out.print("testing read timeout... ");
        out.flush();
        {
            //
            // Expect TimeoutException.
            //
            TimeoutPrx to = TimeoutPrxHelper.uncheckedCast(obj.ice_timeout(500));
            try
            {
                to.sleep(750);
                test(false);
            }
            catch(Ice.TimeoutException ex)
            {
                // Expected.
            }
        }
        {
            //
            // Expect success.
            //
            timeout.op(); // Ensure adapter is active.
            TimeoutPrx to = TimeoutPrxHelper.uncheckedCast(obj.ice_timeout(1500));
            try
            {
                to.sleep(500);
            }
            catch(Ice.TimeoutException ex)
            {
                test(false);
            }
        }
        out.println("ok");

        out.print("testing write timeout... ");
        out.flush();
        {
            //
            // Expect TimeoutException.
            //
            TimeoutPrx to = TimeoutPrxHelper.uncheckedCast(obj.ice_timeout(500));
            to.holdAdapter(2000);
            try
            {
                byte[] seq = new byte[100000];
                to.sendData(seq);
                test(false);
            }
            catch(Ice.TimeoutException ex)
            {
                // Expected.
            }
        }
        {
            //
            // Expect success.
            //
            timeout.op(); // Ensure adapter is active.
            TimeoutPrx to = TimeoutPrxHelper.uncheckedCast(obj.ice_timeout(1500));
            to.holdAdapter(500);
            try
            {
                byte[] seq = new byte[100000];
                to.sendData(seq);
            }
            catch(Ice.TimeoutException ex)
            {
                test(false);
            }
        }
        out.println("ok");

        out.print("testing AMI read timeout... ");
        out.flush();
        {
            //
            // The resolution of AMI timeouts is limited by the connection monitor
            // thread. We set Ice.MonitorConnections=1 (one second) in main().
            //
            // Expect TimeoutException.
            //
            TimeoutPrx to = TimeoutPrxHelper.uncheckedCast(obj.ice_timeout(500));
            AMISleepEx cb = new AMISleepEx();
            to.sleep_async(cb, 2000);
            test(cb.check());
        }
        {
            //
            // Expect success.
            //
            timeout.op(); // Ensure adapter is active.
            TimeoutPrx to = TimeoutPrxHelper.uncheckedCast(obj.ice_timeout(1500));
            AMISleep cb = new AMISleep();
            to.sleep_async(cb, 500);
            test(cb.check());
        }
        out.println("ok");

        out.print("testing AMI write timeout... ");
        out.flush();
        {
            //
            // The resolution of AMI timeouts is limited by the connection monitor
            // thread. We set Ice.MonitorConnections=1 (one second) in main().
            //
            // Expect TimeoutException.
            //
            TimeoutPrx to = TimeoutPrxHelper.uncheckedCast(obj.ice_timeout(500));
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
            TimeoutPrx to = TimeoutPrxHelper.uncheckedCast(obj.ice_timeout(1500));
            to.holdAdapter(500);
            byte[] seq = new byte[100000];
            AMISendData cb = new AMISendData();
            to.sendData_async(cb, seq);
            test(cb.check());
        }
        out.println("ok");

        out.print("testing timeout overrides... ");
        out.flush();
        {
            //
            // Test Ice.Override.Timeout. This property overrides all
            // endpoint timeouts.
            //
            String[] args = new String[0];
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = communicator.getProperties()._clone();
            initData.properties.setProperty("Ice.Override.Timeout", "500");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);
            TimeoutPrx to = TimeoutPrxHelper.checkedCast(comm.stringToProxy(sref));
            try
            {
                to.sleep(750);
                test(false);
            }
            catch(Ice.TimeoutException ex)
            {
                // Expected.
            }
            //
            // Calling ice_timeout() should have no effect.
            //
            timeout.op(); // Ensure adapter is active.
            to = TimeoutPrxHelper.checkedCast(to.ice_timeout(1000));
            try
            {
                to.sleep(750);
                test(false);
            }
            catch(Ice.TimeoutException ex)
            {
                // Expected.
            }
            comm.destroy();
        }
        {
            //
            // Test Ice.Override.ConnectTimeout.
            //
            String[] args = new String[0];
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = communicator.getProperties()._clone();
            initData.properties.setProperty("Ice.Override.ConnectTimeout", "1000");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);
            TimeoutPrx to = TimeoutPrxHelper.uncheckedCast(comm.stringToProxy(sref));
            timeout.holdAdapter(3000);
            try
            {
                to.op();
                test(false);
            }
            catch(Ice.ConnectTimeoutException ex)
            {
                // Expected.
            }
            //
            // Calling ice_timeout() should have no effect on the connect timeout.
            //
            timeout.op(); // Ensure adapter is active.
            timeout.holdAdapter(3000);
            to = TimeoutPrxHelper.uncheckedCast(to.ice_timeout(3500));
            try
            {
                to.op();
                test(false);
            }
            catch(Ice.ConnectTimeoutException ex)
            {
                // Expected.
            }
            //
            // Verify that timeout set via ice_timeout() is still used for requests.
            //
            timeout.op(); // Ensure adapter is active.
            to.op(); // Force connection.
            try
            {
                to.sleep(4000);
                test(false);
            }
            catch(Ice.TimeoutException ex)
            {
                // Expected.
            }
            comm.destroy();
        }
        out.println("ok");

        return timeout;
    }
}
