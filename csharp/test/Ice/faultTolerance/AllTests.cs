//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
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
            catch
            {
                output.WriteLine(ex.ToString());
                TestHelper.Assert(false);
            }
        }

        public static void Run(TestHelper helper, List<int> ports)
        {
            Communicator? communicator = helper.Communicator();
            TestHelper.Assert(communicator != null);
            TextWriter output = helper.GetWriter();
            output.Write("testing stringToProxy... ");
            output.Flush();

            // Build a multi-endpoint proxy by hand.
            // TODO: should the TestHelper help with that?
            string refString = helper.GetTestProxy("test", 0);
            if (ports.Count > 1)
            {
                var sb = new StringBuilder(refString);
                if (helper.GetTestProtocol() == Protocol.Ice1)
                {
                    string transport = helper.GetTestTransport();
                    for (int i = 1; i < ports.Count; ++i)
                    {
                        sb.Append($": {transport} -h ");
                        sb.Append(helper.GetTestHost());
                        sb.Append(" -p ");
                        sb.Append(helper.GetTestPort(ports[i]));
                    }
                }
                else
                {
                    sb.Append("?alt-endpoint=");
                    for (int i = 1; i < ports.Count; ++i)
                    {
                        if (i > 1)
                        {
                            sb.Append(',');
                        }
                        sb.Append(helper.GetTestHost());
                        sb.Append(':');
                        sb.Append(helper.GetTestPort(ports[i]));
                    }
                }
                refString = sb.ToString();
            }

            var obj = ITestIntfPrx.Parse(refString, communicator);
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
                        output.Write($"aborting server #{i} with AMI... ");
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
                        output.Write($"aborting server #{i} and #{i + 1} with idempotent call... ");
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
                        output.Write($"aborting server #{i} and #{i + 1} with idempotent AMI call... ");
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
            catch
            {
                output.WriteLine("ok");
            }
        }
    }
}
