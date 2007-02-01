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

        public synchronized boolean
        check()
        {
            while(!_called)
            {
                try
                {
                    wait(30000);
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

    private static class AMI_Test_pidI extends AMI_TestIntf_pid
    {
        public void
        ice_response(int pid)
        {
            _pid = pid;
            callback.called();
        }
        
        public void
        ice_exception(Ice.LocalException ex)
        {
            test(false);
        }
        
        public void
        ice_exception(Ice.UserException ex)
        {
            test(false);
        }
        
        public int
        pid()
        {
            return _pid;
        }
        
        public boolean
        check()
        {
            return callback.check();
        }

        private int _pid;
        
        private Callback callback = new Callback();
    }
    
    private static class AMI_Test_shutdownI extends AMI_TestIntf_shutdown
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
        
        public void
        ice_exception(Ice.UserException ex)
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
    
    private static class AMI_Test_abortI extends AMI_TestIntf_abort
    {
        public void
        ice_response()
        {
            test(false);
        }
        
        public void
        ice_exception(Ice.LocalException ex)
        {
            try
            {
                throw ex;
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
        ice_exception(Ice.UserException ex)
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
    
    private static class AMI_Test_idempotentAbortI extends AMI_TestIntf_idempotentAbort
    {
        public void
        ice_response()
        {
            test(false);
        }
        
        public void
        ice_exception(Ice.LocalException ex)
        {
            delegate.ice_exception(ex);
        }
        
        public void
        ice_exception(Ice.UserException ex)
        {
            delegate.ice_exception(ex);
        }
        
        public boolean
        check()
        {
            return delegate.check();
        }

        private AMI_Test_abortI delegate = new AMI_Test_abortI();
    }
     
    public static void
    allTests(Ice.Communicator communicator, int[] ports)
    {
        System.out.print("testing stringToProxy... ");
        System.out.flush();
        String ref = "test";
        for(int i = 0; i < ports.length; i++)
        {
            ref += ":default -t 60000 -p " + ports[i];
        }
        Ice.ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);
        System.out.println("ok");

        System.out.print("testing checked cast... ");
        System.out.flush();
        TestIntfPrx obj = TestIntfPrxHelper.checkedCast(base);
        test(obj != null);
        test(obj.equals(base));
        System.out.println("ok");

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
                System.out.print("testing server #" + i + "... ");
                System.out.flush();
                int pid = obj.pid();
                test(pid != oldPid);
                System.out.println("ok");
                oldPid = pid;
            }
            else
            {
                System.out.print("testing server #" + i + " with AMI... ");
                System.out.flush();
                AMI_Test_pidI cb = new AMI_Test_pidI();
                obj.pid_async(cb);
                test(cb.check());
                int pid = cb.pid();
                test(pid != oldPid);
                System.out.println("ok");
                oldPid = pid;
            }

            if(j == 0)
            {
                if(!ami)
                {
                    System.out.print("shutting down server #" + i + "... ");
                    System.out.flush();
                    obj.shutdown();
                    System.out.println("ok");
                }
                else
                {
                    System.out.print("shutting down server #" + i + " with AMI... ");
                    System.out.flush();
                    AMI_Test_shutdownI cb = new AMI_Test_shutdownI();
                    obj.shutdown_async(cb);
                    test(cb.check());
                    System.out.println("ok");
                }
            }
            else if(j == 1 || i + 1 > ports.length)
            {
                if(!ami)
                {
                    System.out.print("aborting server #" + i + "... ");
                    System.out.flush();
                    try
                    {
                        obj.abort();
                        test(false);
                    }
                    catch(Ice.ConnectionLostException ex)
                    {
                        System.out.println("ok");
                    }
                    catch(Ice.ConnectFailedException exc)
                    {
                        System.out.println("ok");
                    }
                    catch(Ice.SocketException ex)
                    {
                        System.out.println("ok");
                    }
                }
                else
                {
                    System.out.print("aborting server #" + i + " with AMI... ");
                    System.out.flush();
                    AMI_Test_abortI cb = new AMI_Test_abortI();
                    obj.abort_async(cb);
                    test(cb.check());
                    System.out.println("ok");
                }
            }
            else if(j == 2 || j == 3)
            {
                if(!ami)
                {
                    System.out.print("aborting server #" + i + " and #" + (i + 1) + " with idempotent call... ");
                    System.out.flush();
                    try
                    {
                        obj.idempotentAbort();
                        test(false);
                    }
                    catch(Ice.ConnectionLostException ex)
                    {
                        System.out.println("ok");
                    }
                    catch(Ice.ConnectFailedException exc)
                    {
                        System.out.println("ok");
                    }
                    catch(Ice.SocketException ex)
                    {
                        System.out.println("ok");
                    }
                }
                else
                {
                    System.out.print("aborting server #" + i + " and #" + (i + 1) + " with idempotent AMI call... ");
                    System.out.flush();
                    AMI_Test_idempotentAbortI cb = new AMI_Test_idempotentAbortI();
                    obj.idempotentAbort_async(cb);
                    test(cb.check());
                    System.out.println("ok");
                }

                ++i;
            }
            else
            {
                assert(false);
            }
        }

        System.out.print("testing whether all servers are gone... ");
        System.out.flush();
        try
        {
            obj.ice_ping();
            test(false);
        }
        catch(Ice.LocalException ex)
        {
            System.out.println("ok");
        }
    }
}
