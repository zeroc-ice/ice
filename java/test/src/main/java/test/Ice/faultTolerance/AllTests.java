// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.faultTolerance;

import java.io.PrintWriter;

import test.Ice.faultTolerance.Test.TestIntfPrx;

public class AllTests
{
    public static void test(boolean b)
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

    private static class PidCallback extends Callback
    {
        public void response(int pid)
        {
            _pid = pid;
            called();
        }

        public int pid()
        {
            return _pid;
        }

        private int _pid;
    }

    private static class AbortCallback extends Callback
    {
        public void completed(Throwable ex)
        {
            try
            {
                throw ex;
            }
            catch(com.zeroc.Ice.ConnectionLostException exc)
            {
            }
            catch(com.zeroc.Ice.ConnectFailedException exc)
            {
            }
            catch(com.zeroc.Ice.SocketException exc)
            {
            }
            catch(Throwable exc)
            {
                test(false);
            }
            called();
        }
    }

    public static void allTests(com.zeroc.Ice.Communicator communicator, int[] ports, PrintWriter out)
    {
        out.print("testing stringToProxy... ");
        out.flush();
        String ref = "test";
        for(int port : ports)
        {
            ref += ":default -p " + port;
        }
        com.zeroc.Ice.ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);
        out.println("ok");

        out.print("testing checked cast... ");
        out.flush();
        TestIntfPrx obj = TestIntfPrx.checkedCast(base);
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
                PidCallback cb = new PidCallback();
                obj.pidAsync().whenComplete((result, ex) ->
                    {
                        test(ex == null);
                        cb.response(result);
                    });
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
                    Callback cb = new Callback();
                    obj.shutdownAsync().whenComplete((result, ex) ->
                        {
                            test(ex == null);
                            cb.called();
                        });
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
                    catch(com.zeroc.Ice.ConnectionLostException ex)
                    {
                        out.println("ok");
                    }
                    catch(com.zeroc.Ice.ConnectFailedException exc)
                    {
                        out.println("ok");
                    }
                    catch(com.zeroc.Ice.SocketException ex)
                    {
                        out.println("ok");
                    }
                }
                else
                {
                    out.print("aborting server #" + i + " with AMI... ");
                    out.flush();
                    AbortCallback cb = new AbortCallback();
                    obj.abortAsync().whenComplete((result, ex) ->
                        {
                            test(ex != null);
                            cb.completed(ex);
                        });
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
                    catch(com.zeroc.Ice.ConnectionLostException ex)
                    {
                        out.println("ok");
                    }
                    catch(com.zeroc.Ice.ConnectFailedException exc)
                    {
                        out.println("ok");
                    }
                    catch(com.zeroc.Ice.SocketException ex)
                    {
                        out.println("ok");
                    }
                }
                else
                {
                    out.print("aborting server #" + i + " and #" + (i + 1) + " with idempotent AMI call... ");
                    out.flush();
                    AbortCallback cb = new AbortCallback();
                    obj.idempotentAbortAsync().whenComplete((result, ex) ->
                        {
                            test(ex != null);
                            cb.completed(ex);
                        });
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
        catch(com.zeroc.Ice.LocalException ex)
        {
            out.println("ok");
        }
    }
}
