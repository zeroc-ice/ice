// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;
using System.Threading;

#if SILVERLIGHT
using System.Windows.Controls;
#endif

public class AllTests : TestCommon.TestApp
{
#if SILVERLIGHT
    public override Ice.InitializationData initData()
    {
        Ice.InitializationData initData = new Ice.InitializationData();
        initData.properties = Ice.Util.createProperties();
        return initData;
    }

    override
    public void run(Ice.Communicator communicator)
#else
    public static void allTests(Ice.Communicator communicator)
#endif
    {
        string sref = "test:default -p 12010";
        Ice.ObjectPrx obj = communicator.stringToProxy(sref);
        test(obj != null);

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
            Ice.IPConnectionInfo info = (Ice.IPConnectionInfo)testPrx.ice_getConnection().getInfo();
            test(info.remotePort == 12030 || info.remotePort == 12031); // make sure we are connected to the proxy port.
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
