//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace ZeroC.Ice.Test.NetworkProxy
{
    public class AllTests
    {
        public static void allTests(TestHelper helper)
        {
            Communicator? communicator = helper.Communicator();
            TestHelper.Assert(communicator != null);
            string sref = "test:" + helper.GetTestEndpoint(0);
            var testPrx = ITestIntfPrx.Parse(sref, communicator);

            int proxyPort = communicator.GetPropertyAsInt("Ice.HTTPProxyPort") ??
                            communicator.GetPropertyAsInt("Ice.SOCKSProxyPort") ?? 0;

            var output = helper.GetWriter();
            output.Write("testing connection... ");
            output.Flush();
            {
                testPrx.IcePing();
            }
            output.WriteLine("ok");

            output.Write("testing connection information... ");
            output.Flush();
            {
                var connection = (IPConnection)testPrx.GetConnection()!;
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
                catch (System.Exception)
                {
                }
            }
            output.WriteLine("ok");
        }
    }
}
