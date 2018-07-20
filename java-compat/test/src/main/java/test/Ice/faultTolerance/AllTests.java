// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.faultTolerance;

import java.io.PrintWriter;

import test.Ice.faultTolerance.Test.TestIntfPrx;
import test.Ice.faultTolerance.Test.TestIntfPrxHelper;
import test.Ice.faultTolerance.Test.Callback_TestIntf_pid;
import test.Ice.faultTolerance.Test.Callback_TestIntf_shutdown;

public class AllTests
{
    public static void
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

    private static class Callback_TestIntf_pidI extends Callback_TestIntf_pid
    {
        @Override
        public void
        response(int pid)
        {
            _pid = pid;
            callback.called();
        }

        @Override
        public void
        exception(Ice.LocalException ex)
        {
            test(false);
        }

        public int
        pid()
        {
            return _pid;
        }

        public void
        check()
        {
            callback.check();
        }

        private int _pid;

        private Callback callback = new Callback();
    }

    private static class Callback_TestIntf_shutdownI extends Callback_TestIntf_shutdown
    {
        @Override
        public void
        response()
        {
            callback.called();
        }

        @Override
        public void
        exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AbortCallback extends Ice.Callback
    {
        @Override
        public void
        completed(Ice.AsyncResult result)
        {
            try
            {
                TestIntfPrx p = TestIntfPrxHelper.uncheckedCast(result.getProxy());
                if(result.getOperation().equals("abort"))
                {
                    p.end_abort(result);
                }
                else if(result.getOperation().equals("idempotentAbort"))
                {
                    p.end_idempotentAbort(result);
                }
                test(false);
            }
            catch(Ice.ConnectionLostException exc)
            {
            }
            catch(Ice.ConnectFailedException exc)
            {
            }
            catch(Ice.SocketException exc)
            {
            }
            catch(Exception exc)
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

    public static void
    allTests(test.TestHelper helper, int[] ports)
    {
        Ice.Communicator communicator = helper.communicator();
        PrintWriter out = helper.getWriter();

        out.print("testing stringToProxy... ");
        out.flush();
        String ref = "test";
        for(int port : ports)
        {
            ref += ":" + helper.getTestEndpoint(port);
        }
        Ice.ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);
        out.println("ok");

        out.print("testing checked cast... ");
        out.flush();
        TestIntfPrx obj = TestIntfPrxHelper.checkedCast(base);
        test(obj != null);
        test(obj.equals(base));
        out.println("ok");

        int oldPid = 0;
        boolean ami = false;
        for(int i = 1, j = 0; i <= ports.length; ++i, ++j)
        {
            if(j > 3)
            {
                j = 0;
                ami = !ami;
            }

            if(!ami)
            {
                out.print("testing server #" + i + "... ");
                out.flush();
                int pid = obj.pid();
                test(pid != oldPid);
                out.println("ok");
                oldPid = pid;
            }
            else
            {
                out.print("testing server #" + i + " with AMI... ");
                out.flush();
                Callback_TestIntf_pidI cb = new Callback_TestIntf_pidI();
                obj.begin_pid(cb);
                cb.check();
                int pid = cb.pid();
                test(pid != oldPid);
                out.println("ok");
                oldPid = pid;
            }

            if(j == 0)
            {
                if(!ami)
                {
                    out.print("shutting down server #" + i + "... ");
                    out.flush();
                    obj.shutdown();
                    out.println("ok");
                }
                else
                {
                    out.print("shutting down server #" + i + " with AMI... ");
                    out.flush();
                    Callback_TestIntf_shutdownI cb = new Callback_TestIntf_shutdownI();
                    obj.begin_shutdown(cb);
                    cb.check();
                    out.println("ok");
                }
            }
            else if(j == 1 || i + 1 > ports.length)
            {
                if(!ami)
                {
                    out.print("aborting server #" + i + "... ");
                    out.flush();
                    try
                    {
                        obj.abort();
                        test(false);
                    }
                    catch(Ice.ConnectionLostException ex)
                    {
                        out.println("ok");
                    }
                    catch(Ice.ConnectFailedException exc)
                    {
                        out.println("ok");
                    }
                    catch(Ice.SocketException ex)
                    {
                        out.println("ok");
                    }
                }
                else
                {
                    out.print("aborting server #" + i + " with AMI... ");
                    out.flush();
                    AbortCallback cb = new AbortCallback();
                    obj.begin_abort(cb);
                    cb.check();
                    out.println("ok");
                }
            }
            else if(j == 2 || j == 3)
            {
                if(!ami)
                {
                    out.print("aborting server #" + i + " and #" + (i + 1) + " with idempotent call... ");
                    out.flush();
                    try
                    {
                        obj.idempotentAbort();
                        test(false);
                    }
                    catch(Ice.ConnectionLostException ex)
                    {
                        out.println("ok");
                    }
                    catch(Ice.ConnectFailedException exc)
                    {
                        out.println("ok");
                    }
                    catch(Ice.SocketException ex)
                    {
                        out.println("ok");
                    }
                }
                else
                {
                    out.print("aborting server #" + i + " and #" + (i + 1) + " with idempotent AMI call... ");
                    out.flush();
                    AbortCallback cb = new AbortCallback();
                    obj.begin_idempotentAbort(cb);
                    cb.check();
                    out.println("ok");
                }

                ++i;
            }
            else
            {
                assert(false);
            }
        }

        out.print("testing whether all servers are gone... ");
        out.flush();
        try
        {
            obj.ice_ping();
            test(false);
        }
        catch(Ice.LocalException ex)
        {
            out.println("ok");
        }
    }
}
