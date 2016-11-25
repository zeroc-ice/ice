// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;
using System.Collections.Generic;
using Test;

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
                    System.Threading.Monitor.Wait(this);
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
                System.Threading.Monitor.Pulse(this);
            }
        }

        private bool _called;
    }

    private static void exceptAbortI(Ice.Exception ex)
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
            WriteLine(ex.ToString());
            test(false);
        }
    }

    public static void allTests(TestCommon.Application app, List<int> ports)
    {
        Ice.Communicator communicator = app.communicator();
        Write("testing stringToProxy... ");
        Flush();
        string refString = "test";
        for(int i = 0; i < ports.Count; i++)
        {
            refString += ":" + app.getTestEndpoint(ports[i]);
        }
        Ice.ObjectPrx basePrx = communicator.stringToProxy(refString);
        test(basePrx != null);
        WriteLine("ok");

        Write("testing checked cast... ");
        Flush();
        TestIntfPrx obj = TestIntfPrxHelper.checkedCast(basePrx);
        test(obj != null);
        test(obj.Equals(basePrx));
        WriteLine("ok");

        if(IceInternal.AssemblyUtil.runtime_ == IceInternal.AssemblyUtil.Runtime.Mono)
        {
            WriteLine("");
            WriteLine("This test aborts a number of server processes.");
            WriteLine("Test output may be interspersed with \"killed\" message from the shell.");
            WriteLine("These messages are expected and do NOT indicate a test failure.");
            WriteLine("");
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
                Write("testing server #" + i + "... ");
                Flush();
                int pid = obj.pid();
                test(pid != oldPid);
                WriteLine("ok");
                oldPid = pid;
            }
            else
            {
                Write("testing server #" + i + " with AMI... ");
                Flush();
                Callback cb = new Callback();
                int pid = -1;
                obj.begin_pid().whenCompleted(
                    (int p) =>
                    {
                        pid = p;
                        cb.called();
                    },
                    (Ice.Exception ex) =>
                    {
                        WriteLine(ex.ToString());
                        test(false);
                    });
                cb.check();
                test(pid != oldPid);
                WriteLine("ok");
                oldPid = pid;
            }

            if(j == 0)
            {
                if(!ami)
                {
                    Write("shutting down server #" + i + "... ");
                    Flush();
                    obj.shutdown();
                    WriteLine("ok");
                }
                else
                {
                    Write("shutting down server #" + i + " with AMI... ");
                    Callback cb = new Callback();
                    obj.begin_shutdown().whenCompleted(
                        () =>
                        {
                            cb.called();
                        },
                        (Ice.Exception ex) =>
                        {
                            WriteLine(ex.ToString());
                            test(false);
                        });
                    cb.check();
                    WriteLine("ok");
                }
            }
            else if(j == 1 || i + 1 > ports.Count)
            {
                if(!ami)
                {
                    Write("aborting server #" + i + "... ");
                    Flush();
                    try
                    {
                        obj.abort();
                        test(false);
                    }
                    catch(Ice.ConnectionLostException)
                    {
                        WriteLine("ok");
                    }
                    catch(Ice.ConnectFailedException)
                    {
                        WriteLine("ok");
                    }
                    catch(Ice.SocketException)
                    {
                        WriteLine("ok");
                    }
                }
                else
                {
                    Write("aborting server #" + i + " with AMI... ");
                    Flush();
                    Callback cb = new Callback();
                    obj.begin_abort().whenCompleted(
                        () =>
                        {
                            test(false);
                        },
                        (Ice.Exception ex) =>
                        {
                            exceptAbortI(ex);
                            cb.called();
                        });
                    cb.check();
                    WriteLine("ok");
                }
            }
            else if(j == 2 || j == 3)
            {
                if(!ami)
                {
                    Write("aborting server #" + i + " and #" + (i + 1) + " with idempotent call... ");
                    Flush();
                    try
                    {
                        obj.idempotentAbort();
                        test(false);
                    }
                    catch(Ice.ConnectionLostException)
                    {
                        WriteLine("ok");
                    }
                    catch(Ice.ConnectFailedException)
                    {
                        WriteLine("ok");
                    }
                    catch(Ice.SocketException)
                    {
                        WriteLine("ok");
                    }
                }
                else
                {
                    Write("aborting server #" + i + " and #" + (i + 1) + " with idempotent AMI call... ");
                    Flush();
                    Callback cb = new Callback();
                    obj.begin_idempotentAbort().whenCompleted(
                        () =>
                        {
                            test(false);
                        },
                        (Ice.Exception ex) =>
                        {
                            exceptAbortI(ex);
                            cb.called();
                        });
                    cb.check();
                    WriteLine("ok");
                }
                ++i;
            }
            else
            {
                Debug.Assert(false);
            }
        }

        Write("testing whether all servers are gone... ");
        Flush();
        try
        {
            obj.ice_ping();
            test(false);
        }
        catch(Ice.LocalException)
        {
            WriteLine("ok");
        }
    }
}
