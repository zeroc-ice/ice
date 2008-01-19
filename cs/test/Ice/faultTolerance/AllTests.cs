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
using Test;

public class AllTests
{
    private static void test(bool b)
    {
        if(!b)
        {
            throw new System.Exception();
        }
    }

    private class Callback
    {
        public Callback()
        {
            _called = false;
        }

        public bool check()
        {
            lock(this)
            {
                while(!_called)
                {
                    Monitor.Wait(this, TimeSpan.FromMilliseconds(30000));

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

    private class AMI_Test_pidI : AMI_TestIntf_pid
    {
        public override void ice_response(int pid)
        {
            _pid = pid;
            callback.called();
        }
        
        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }
        
        public int pid()
        {
            return _pid;
        }
        
        public bool check()
        {
            return callback.check();
        }

        private int _pid;
        
        private Callback callback = new Callback();
    }
    
    private class AMI_Test_shutdownI : AMI_TestIntf_shutdown
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
    
    private class AMI_Test_abortI : AMI_TestIntf_abort
    {
        public override void ice_response()
        {
            test(false);
        }
        
        public override void ice_exception(Ice.Exception ex)
        {
            try
            {
                throw ex;
            }
            catch(Ice.ConnectionLostException )
            {
            }
            catch(Ice.ConnectFailedException)
            {
            }
            catch(Ice.SocketException)
            {
            }
            catch(Exception)
            {
                test(false);
            }
            callback.called();
        }
        
        public bool check()
        {
            return callback.check();
        }

        private Callback callback = new Callback();
    }
    
    private class AMI_Test_idempotentAbortI : AMI_TestIntf_idempotentAbort
    {
        public override void ice_response()
        {
            test(false);
        }
        
        public override void ice_exception(Ice.Exception ex)
        {
            @delegate.ice_exception(ex);
        }
        
        public bool check()
        {
            return @delegate.check();
        }

        private AMI_Test_abortI @delegate = new AMI_Test_abortI();
    }
    
    public static void allTests(Ice.Communicator communicator, System.Collections.ArrayList ports)
    {
        Console.Out.Write("testing stringToProxy... ");
        Console.Out.Flush();
        string refString = "test";
        for(int i = 0; i < ports.Count; i++)
        {
            refString += ":default -t 60000 -p " + ports[i];
        }
        Ice.ObjectPrx basePrx = communicator.stringToProxy(refString);
        test(basePrx != null);
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("testing checked cast... ");
        Console.Out.Flush();
        TestIntfPrx obj = TestIntfPrxHelper.checkedCast(basePrx);
        test(obj != null);
        test(obj.Equals(basePrx));
        Console.Out.WriteLine("ok");
        
        int oldPid = 0;
        bool ami = false;
        for(int i = 1, j = 0; i <= ports.Count; ++i, ++j)
        {
            if(j > 3)
            {
                j = 0;
                ami = !ami;
            }

            if(!ami)
            {
                Console.Out.Write("testing server #" + i + "... ");
                Console.Out.Flush();
                int pid = obj.pid();
                test(pid != oldPid);
                Console.Out.WriteLine("ok");
                oldPid = pid;
            }
            else
            {
                Console.Out.Write("testing server #" + i + " with AMI... ");
                Console.Out.Flush();
                AMI_Test_pidI cb = new AMI_Test_pidI();
                obj.pid_async(cb);
                test(cb.check());
                int pid = cb.pid();
                test(pid != oldPid);
                Console.Out.WriteLine("ok");
                oldPid = pid;
            }
            
            if(j == 0)
            {
                if(!ami)
                {
                    Console.Out.Write("shutting down server #" + i + "... ");
                    Console.Out.Flush();
                    obj.shutdown();
                    Console.Out.WriteLine("ok");
                }
                else
                {
                    Console.Out.Write("shutting down server #" + i + " with AMI... ");
                    AMI_Test_shutdownI cb = new AMI_Test_shutdownI();
                    obj.shutdown_async(cb);
                    test(cb.check());
                    Console.Out.WriteLine("ok");
                }
            }
            else if(j == 1 || i + 1 > ports.Count)
            {
                if(!ami)
                {
                    Console.Out.Write("aborting server #" + i + "... ");
                    Console.Out.Flush();
                    try
                    {
                        obj.abort();
                        test(false);
                    }
                    catch(Ice.ConnectionLostException)
                    {
                        Console.Out.WriteLine("ok");
                    }
                    catch(Ice.ConnectFailedException)
                    {
                        Console.Out.WriteLine("ok");
                    }
                    catch(Ice.SocketException)
                    {
                        Console.Out.WriteLine("ok");
                    }
                }
                else
                {
                    Console.Out.Write("aborting server #" + i + " with AMI... ");
                    Console.Out.Flush();
                    AMI_Test_abortI cb = new AMI_Test_abortI();
                    obj.abort_async(cb);
                    test(cb.check());
                    Console.Out.WriteLine("ok");
                }
            }
            else if(j == 2 || j == 3)
            {
                if(!ami)
                {
                    Console.Out.Write("aborting server #" + i + " and #" + (i + 1) + " with idempotent call... ");
                    Console.Out.Flush();
                    try
                    {
                        obj.idempotentAbort();
                        test(false);
                    }
                    catch(Ice.ConnectionLostException)
                    {
                        Console.Out.WriteLine("ok");
                    }
                    catch(Ice.ConnectFailedException)
                    {
                        Console.Out.WriteLine("ok");
                    }
                    catch(Ice.SocketException)
                    {
                        Console.Out.WriteLine("ok");
                    }
                }
                else
                {
                    Console.Out.Write("aborting server #" + i + " and #" + (i + 1) + " with idempotent AMI call... ");
                    Console.Out.Flush();
                    AMI_Test_idempotentAbortI cb = new AMI_Test_idempotentAbortI();
                    obj.idempotentAbort_async(cb);
                    test(cb.check());
                    Console.Out.WriteLine("ok");
                }
                ++i;
            }
            else
            {
                Debug.Assert(false);
            }
        }
        
        Console.Out.Write("testing whether all servers are gone... ");
        Console.Out.Flush();
        try
        {
            obj.ice_ping();
            test(false);
        }
        catch(Ice.LocalException)
        {
            Console.Out.WriteLine("ok");
        }
    }
}
