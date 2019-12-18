//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice;

public class AllTests : Test.AllTests
{
    private static Ice.IPConnectionInfo getIPConnectionInfo(Ice.ConnectionInfo info)
    {
        for (; info != null; info = info.underlying)
        {
            if (info is Ice.IPConnectionInfo)
            {
                return info as Ice.IPConnectionInfo;
            }
        }
        return null;
    }

    public static void allTests(Test.TestHelper helper)
    {
        Ice.Communicator communicator = helper.communicator();
        string sref = "test:" + helper.getTestEndpoint(0);
        var obj = IObjectPrx.Parse(sref, communicator);

        int proxyPort = communicator.GetPropertyAsInt("Ice.HTTPProxyPort") ??
                        communicator.GetPropertyAsInt("Ice.SOCKSProxyPort") ?? 0;

        Test.TestIntfPrx? testPrx = Test.TestIntfPrx.CheckedCast(obj);
        var output = helper.getWriter();
        output.Write("testing connection... ");
        output.Flush();
        {
            testPrx.IcePing();
        }
        output.WriteLine("ok");

        output.Write("testing connection information... ");
        output.Flush();
        {
            Ice.IPConnectionInfo info = getIPConnectionInfo(testPrx.GetConnection().getInfo());
            test(info.remotePort == proxyPort); // make sure we are connected to the proxy port.
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
                test(false);
            }
            catch (Ice.LocalException)
            {
            }
        }
        output.WriteLine("ok");
    }
}
