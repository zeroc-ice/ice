//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Diagnostics;
using System.Collections.Generic;
using System.IO;
using Test;

public class AllTests : Test.AllTests
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

    private static void exceptAbortI(Ice.Exception ex, TextWriter output)
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
            output.WriteLine(ex.ToString());
            test(false);
        }
    }

    public static void allTests(Test.TestHelper helper, List<int> ports)
    {
        Ice.Communicator communicator = helper.communicator();
        var output = helper.getWriter();
        output.Write("testing stringToProxy... ");
        output.Flush();
        string refString = "test";
        for(int i = 0; i < ports.Count; i++)
        {
            refString += ":" + helper.getTestEndpoint(ports[i]);
        }
        Ice.ObjectPrx basePrx = communicator.stringToProxy(refString);
        test(basePrx != null);
        output.WriteLine("ok");

        output.Write("testing checked cast... ");
        output.Flush();
        TestIntfPrx obj = TestIntfPrxHelper.checkedCast(basePrx);
        test(obj != null);
        test(obj.Equals(basePrx));
        output.WriteLine("ok");

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
                output.Write("testing server #" + i + "... ");
                output.Flush();
                int pid = obj.pid();
                test(pid != oldPid);
                output.WriteLine("ok");
                oldPid = pid;
            }
            else
            {
                output.Write("testing server #" + i + " with AMI... ");
                output.Flush();
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
                        output.WriteLine(ex.ToString());
                        test(false);
                    });
                cb.check();
                test(pid != oldPid);
                output.WriteLine("ok");
                oldPid = pid;
            }

            if(j == 0)
            {
                if(!ami)
                {
                    output.Write("shutting down server #" + i + "... ");
                    output.Flush();
                    obj.shutdown();
                    output.WriteLine("ok");
                }
                else
                {
                    output.Write("shutting down server #" + i + " with AMI... ");
                    Callback cb = new Callback();
                    obj.begin_shutdown().whenCompleted(
                        () =>
                        {
                            cb.called();
                        },
                        (Ice.Exception ex) =>
                        {
                            output.WriteLine(ex.ToString());
                            test(false);
                        });
                    cb.check();
                    output.WriteLine("ok");
                }
            }
            else if(j == 1 || i + 1 > ports.Count)
            {
                if(!ami)
                {
                    output.Write("aborting server #" + i + "... ");
                    output.Flush();
                    try
                    {
                        obj.abort();
                        test(false);
                    }
                    catch(Ice.ConnectionLostException)
                    {
                        output.WriteLine("ok");
                    }
                    catch(Ice.ConnectFailedException)
                    {
                        output.WriteLine("ok");
                    }
                    catch(Ice.SocketException)
                    {
                        output.WriteLine("ok");
                    }
                }
                else
                {
                    output.Write("aborting server #" + i + " with AMI... ");
                    output.Flush();
                    Callback cb = new Callback();
                    obj.begin_abort().whenCompleted(
                        () =>
                        {
                            test(false);
                        },
                        (Ice.Exception ex) =>
                        {
                            exceptAbortI(ex, output);
                            cb.called();
                        });
                    cb.check();
                    output.WriteLine("ok");
                }
            }
            else if(j == 2 || j == 3)
            {
                if(!ami)
                {
                    output.Write("aborting server #" + i + " and #" + (i + 1) + " with idempotent call... ");
                    output.Flush();
                    try
                    {
                        obj.idempotentAbort();
                        test(false);
                    }
                    catch(Ice.ConnectionLostException)
                    {
                        output.WriteLine("ok");
                    }
                    catch(Ice.ConnectFailedException)
                    {
                        output.WriteLine("ok");
                    }
                    catch(Ice.SocketException)
                    {
                        output.WriteLine("ok");
                    }
                }
                else
                {
                    output.Write("aborting server #" + i + " and #" + (i + 1) + " with idempotent AMI call... ");
                    output.Flush();
                    Callback cb = new Callback();
                    obj.begin_idempotentAbort().whenCompleted(
                        () =>
                        {
                            test(false);
                        },
                        (Ice.Exception ex) =>
                        {
                            exceptAbortI(ex, output);
                            cb.called();
                        });
                    cb.check();
                    output.WriteLine("ok");
                }
                ++i;
            }
            else
            {
                Debug.Assert(false);
            }
        }

        output.Write("testing whether all servers are gone... ");
        output.Flush();
        try
        {
            obj.ice_ping();
            test(false);
        }
        catch(Ice.LocalException)
        {
            output.WriteLine("ok");
        }
    }
}
