//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice;
using Test;

public class AllTests
{
    private static IPConnectionInfo? getIPConnectionInfo(ConnectionInfo info)
    {
        ConnectionInfo? underlying = info;
        for (; underlying != null; underlying = underlying.Underlying)
        {
            if (info is IPConnectionInfo)
            {
                return (IPConnectionInfo)underlying;
            }
        }
        return null;
    }

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
            IPConnectionInfo? info = getIPConnectionInfo(testPrx.GetConnection().GetConnectionInfo());
            TestHelper.Assert(info!.RemotePort == proxyPort); // make sure we are connected to the proxy port.
        }
        output.WriteLine("ok");

        output.Write("shutting down server... ");
        output.Flush();
        {
            testPrx.shutdown();
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
