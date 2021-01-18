// Copyright (c) ZeroC, Inc. All rights reserved.

using System.IO;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.NetworkProxy
{
    public static class AllTests
    {
        public static async Task RunAsync(TestHelper helper)
        {
            Communicator communicator = helper.Communicator;

            string sref = helper.GetTestProxy("test", 0);
            var testPrx = ITestIntfPrx.Parse(sref, communicator);

            int proxyPort = communicator.GetPropertyAsInt("Ice.HTTPProxyPort") ??
                            communicator.GetPropertyAsInt("Ice.SOCKSProxyPort") ?? 0;

            TextWriter output = helper.Output;
            output.Write("testing connection... ");
            output.Flush();
            {
                testPrx.IcePing();
            }
            output.WriteLine("ok");

            output.Write("testing connection information... ");
            output.Flush();
            {
                var connection = (IPConnection)await testPrx.GetConnectionAsync();
                TestHelper.Assert(connection.RemoteEndpoint!.Port == proxyPort); // make sure we are connected to the proxy port.
            }
            output.WriteLine("ok");

            output.Write("shutting down server... ");
            output.Flush();
            {
                testPrx.Shutdown();
            }
            output.WriteLine("ok");

            output.Write("testing connection failure... ");
            output.Flush();
            {
                try
                {
                    testPrx.IcePing();
                    TestHelper.Assert(false);
                }
                catch
                {
                }
            }
            output.WriteLine("ok");
        }
    }
}
