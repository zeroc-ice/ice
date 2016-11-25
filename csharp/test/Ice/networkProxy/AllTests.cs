// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public class AllTests : TestCommon.AllTests
{
    private static Ice.IPConnectionInfo getIPConnectionInfo(Ice.ConnectionInfo info)
    {
        for(; info != null; info = info.underlying)
        {
            if(info is Ice.IPConnectionInfo)
            {
                return info as Ice.IPConnectionInfo;
            }
        }
        return null;
    }

    public static void allTests(TestCommon.Application app)
    {
        Ice.Communicator communicator = app.communicator();
        string sref = "test:" + app.getTestEndpoint(0);
        Ice.ObjectPrx obj = communicator.stringToProxy(sref);
        test(obj != null);

        int proxyPort = communicator.getProperties().getPropertyAsInt("Ice.HTTPProxyPort");
        if(proxyPort == 0)
        {
            proxyPort = communicator.getProperties().getPropertyAsInt("Ice.SOCKSProxyPort");
        }

        Test.TestIntfPrx testPrx = Test.TestIntfPrxHelper.checkedCast(obj);
        test(testPrx != null);

        Write("testing connection... ");
        Flush();
        {
            testPrx.ice_ping();
        }
        WriteLine("ok");

        Write("testing connection information... ");
        Flush();
        {
            Ice.IPConnectionInfo info = getIPConnectionInfo(testPrx.ice_getConnection().getInfo());
            test(info.remotePort == proxyPort); // make sure we are connected to the proxy port.
        }
        WriteLine("ok");

        Write("shutting down server... ");
        Flush();
        {
            testPrx.shutdown();
        }
        WriteLine("ok");

        Write("testing connection failure... ");
        Flush();
        {
            try
            {
                testPrx.ice_ping();
                test(false);
            }
            catch(Ice.LocalException)
            {
            }
        }
        WriteLine("ok");
    }
}
