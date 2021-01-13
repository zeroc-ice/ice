// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.FaultTolerance
{
    public static class AllTests
    {
        public static Task RunAsync(TestHelper helper, List<int> ports)
        {
            Communicator communicator = helper.Communicator;

            TextWriter output = helper.Output;
            output.Write("testing stringToProxy... ");
            output.Flush();

            // Build a multi-endpoint proxy by hand.
            // TODO: should the TestHelper help with that?
            string refString = helper.GetTestProxy("test", 0);
            if (ports.Count > 1)
            {
                var sb = new StringBuilder(refString);
                if (helper.Protocol == Protocol.Ice1)
                {
                    string transport = helper.Transport;
                    for (int i = 0; i < ports.Count; ++i)
                    {
                        sb.Append($": {transport} -h ");
                        sb.Append(helper.Host.Contains(":") ? $"\"{helper.Host}\"" : helper.Host);
                        sb.Append(" -p ");
                        sb.Append(helper.BasePort + ports[i]);
                    }
                }
                else
                {
                    sb.Append("?alt-endpoint=");
                    for (int i = 0; i < ports.Count; ++i)
                    {
                        if (i > 0)
                        {
                            sb.Append(',');
                        }
                        sb.Append(helper.Host.Contains(":") ? $"[{helper.Host}]" : helper.Host);
                        sb.Append(':');
                        sb.Append(helper.BasePort + ports[i]);
                    }
                }
                refString = sb.ToString();
            }

            var obj = ITestIntfPrx.Parse(refString, communicator);
            output.WriteLine("ok");

            int oldPid = 0;
            for (int i = 1, j = 0; i <= ports.Count; ++i, ++j)
            {
                output.Write($"testing server #{i}... ");
                output.Flush();
                int pid = obj.Pid();
                TestHelper.Assert(pid != oldPid);
                output.WriteLine("ok");
                oldPid = pid;

                if (i % 2 == 0)
                {
                    output.Write($"shutting down server #{i}... ");
                    output.Flush();
                    obj.Clone(invocationTimeout: TimeSpan.FromMilliseconds(100)).Shutdown();
                    output.WriteLine("ok");
                }
                else
                {
                    output.Write($"aborting server #{i}... ");
                    output.Flush();
                    try
                    {
                        obj.Clone(invocationTimeout: TimeSpan.FromMilliseconds(100)).Abort();
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
            }

            output.Write("testing whether all servers are gone... ");
            output.Flush();
            try
            {
                obj.Clone(invocationTimeout: TimeSpan.FromMilliseconds(100)).IcePing();
                TestHelper.Assert(false);
            }
            catch
            {
                output.WriteLine("ok");
            }
            return Task.CompletedTask;
        }
    }
}
