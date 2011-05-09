// **********************************************************************
//
// Copyright (c) 2003-2010 ZeroC, Inc. All rights reserved.
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

    private class CallbackBase
    {
        public CallbackBase()
        {
            _called = false;
        }

        public void check()
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
        
        public void called()
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
            test(false);
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public void opPidI(int pid)
        {
            _pid = pid;
            callback.called();
        }

        public void opShutdownI()
        {
            callback.called();
        }
        
        public void exceptAbortI(Ice.Exception ex)
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

        public int pid()
        {
            return _pid;
        }
        
        public void check()
        {
            callback.check();
        }

        private int _pid;
        
        private CallbackBase callback = new CallbackBase();
    }
    
    public static void allTests(Ice.Communicator communicator, System.Collections.ArrayList ports)
    {
        Console.Out.Write("testing stringToProxy... ");
        Console.Out.Flush();
        string refString = "test";
        for(int i = 0; i < ports.Count; i++)
        {
            refString += ":default -p " + ports[i];
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
        
        if(IceInternal.AssemblyUtil.runtime_ == IceInternal.AssemblyUtil.Runtime.Mono)
        {
            Console.Out.WriteLine();
            Console.Out.WriteLine("This test aborts a number of server processes.");
            Console.Out.WriteLine("Test output may be interspersed with \"killed\" message from the shell.");
            Console.Out.WriteLine("These messages are expected and do NOT indicate a test failure.");
            Console.Out.WriteLine();
        }

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
                Callback cb = new Callback();
                obj.begin_pid().whenCompleted(cb.opPidI, cb.exception);
                cb.check();
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
                    Callback cb = new Callback();
                    obj.begin_shutdown().whenCompleted(cb.opShutdownI, cb.exception);
                    cb.check();
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
                    Callback cb = new Callback();
                    obj.begin_abort().whenCompleted(cb.response, cb.exceptAbortI);
                    cb.check();
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
                    Callback cb = new Callback();
                    obj.begin_idempotentAbort().whenCompleted(cb.response, cb.exceptAbortI);
                    cb.check();
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
