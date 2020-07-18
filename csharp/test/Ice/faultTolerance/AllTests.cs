//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Diagnostics;
using System.Collections.Generic;
using System.IO;
using Test;

namespace ZeroC.Ice.Test.FaultTolerance
{
    public class AllTests
    {
        private static void ExceptAbortI(Exception ex, TextWriter output)
        {
            try
            {
                throw ex;
            }
            catch (ConnectionLostException)
            {
            }
            catch (ConnectFailedException)
            {
            }
            catch (TransportException)
            {
            }
            catch (Exception)
            {
                output.WriteLine(ex.ToString());
                TestHelper.Assert(false);
            }
        }

        public static void allTests(TestHelper helper, List<int> ports)
        {
            Communicator? communicator = helper.Communicator();
            TestHelper.Assert(communicator != null);
            TextWriter output = helper.GetWriter();
            output.Write("testing stringToProxy... ");
            output.Flush();
            string refString = "test";
            for (int i = 0; i < ports.Count; i++)
            {
                refString += ":" + helper.GetTestEndpoint(ports[i]);
            }
            var basePrx = IObjectPrx.Parse(refString, communicator);
            output.WriteLine("ok");

            output.Write("testing checked cast... ");
            output.Flush();
            var obj = ITestIntfPrx.CheckedCast(basePrx);
            TestHelper.Assert(obj != null);
            TestHelper.Assert(obj.Equals(basePrx));
            output.WriteLine("ok");

            int oldPid = 0;
            bool ami = false;
            for (int i = 1, j = 0; i <= ports.Count; ++i, ++j)
            {
                if (j > 3)
                {
                    j = 0;
                    ami = !ami;
                }

                if (!ami)
                {
                    output.Write("testing server #" + i + "... ");
                    output.Flush();
                    int pid = obj.Pid();
                    TestHelper.Assert(pid != oldPid);
                    output.WriteLine("ok");
                    oldPid = pid;
                }
                else
                {
                    output.Write("testing server #" + i + " with AMI... ");
                    output.Flush();
                    int pid = obj.PidAsync().Result;
                    TestHelper.Assert(pid != oldPid);
                    output.WriteLine("ok");
                    oldPid = pid;
                }

                if (j == 0)
                {
                    if (!ami)
                    {
                        output.Write("shutting down server #" + i + "... ");
                        output.Flush();
                        obj.Shutdown();
                        output.WriteLine("ok");
                    }
                    else
                    {
                        output.Write("shutting down server #" + i + " with AMI... ");
                        obj.ShutdownAsync().Wait();
                        output.WriteLine("ok");
                    }
                }
                else if (j == 1 || i + 1 > ports.Count)
                {
                    if (!ami)
                    {
                        output.Write("aborting server #" + i + "... ");
                        output.Flush();
                        try
                        {
                            obj.Abort();
                            TestHelper.Assert(false);
                        }
                        catch (ConnectionLostException)
                        {
                            output.WriteLine("ok");
                        }
                        catch (ConnectFailedException)
                        {
                            output.WriteLine("ok");
                        }
                        catch (TransportException)
                        {
                            output.WriteLine("ok");
                        }
                    }
                    else
                    {
                        output.Write("aborting server #" + i + " with AMI... ");
                        output.Flush();
                        try
                        {
                            obj.AbortAsync().Wait();
                            TestHelper.Assert(false);
                        }
                        catch (AggregateException ex)
                        {
                            TestHelper.Assert(ex.InnerException != null);
                            ExceptAbortI(ex.InnerException, output);
                        }
                        output.WriteLine("ok");
                    }
                }
                else if (j == 2 || j == 3)
                {
                    if (!ami)
                    {
                        output.Write("aborting server #" + i + " and #" + (i + 1) + " with idempotent call... ");
                        output.Flush();
                        try
                        {
                            obj.IdempotentAbort();
                            TestHelper.Assert(false);
                        }
                        catch (ConnectionLostException)
                        {
                            output.WriteLine("ok");
                        }
                        catch (ConnectFailedException)
                        {
                            output.WriteLine("ok");
                        }
                        catch (TransportException)
                        {
                            output.WriteLine("ok");
                        }
                    }
                    else
                    {
                        output.Write("aborting server #" + i + " and #" + (i + 1) + " with idempotent AMI call... ");
                        output.Flush();
                        try
                        {
                            obj.IdempotentAbortAsync().Wait();
                            TestHelper.Assert(false);
                        }
                        catch (AggregateException ex)
                        {
                            TestHelper.Assert(ex.InnerException != null);
                            ExceptAbortI(ex.InnerException, output);
                        }
                        output.WriteLine("ok");
                    }
                    ++i;
                }
                else
                {
                    TestHelper.Assert(false);
                }
            }

            output.Write("testing whether all servers are gone... ");
            output.Flush();
            try
            {
                obj.IcePing();
                TestHelper.Assert(false);
            }
            catch (Exception)
            {
                output.WriteLine("ok");
            }
        }
    }
}
