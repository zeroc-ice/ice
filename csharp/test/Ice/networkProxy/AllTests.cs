// Copyright (c) ZeroC, Inc.

namespace Ice.networkProxy;

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
        Ice.ObjectPrx obj = communicator.stringToProxy(sref);
        test(obj != null);

        int proxyPort = communicator.getProperties().getIcePropertyAsInt("Ice.HTTPProxyPort");
        if (proxyPort == 1080) // default, i.e. most likely not set
        {
            proxyPort = communicator.getProperties().getIcePropertyAsInt("Ice.SOCKSProxyPort");
        }

        Test.TestIntfPrx testPrx = Test.TestIntfPrxHelper.checkedCast(obj);
        test(testPrx != null);
        TextWriter output = helper.getWriter();
        output.Write("testing connection... ");
        output.Flush();
        {
            testPrx.ice_ping();
        }
        output.WriteLine("ok");

        output.Write("testing connection information... ");
        output.Flush();
        {
            Ice.IPConnectionInfo info = getIPConnectionInfo(testPrx.ice_getConnection().getInfo());
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
                testPrx.ice_ping();
                test(false);
            }
            catch (Ice.LocalException)
            {
            }
        }
        output.WriteLine("ok");
    }
}
