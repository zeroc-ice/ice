// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.networkProxy;

import java.io.PrintWriter;

import test.Ice.networkProxy.Test.TestIntfPrx;

public class AllTests
{
    private static void test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    public static void allTests(test.Util.Application app)
    {
        com.zeroc.Ice.Communicator communicator = app.communicator();
        PrintWriter out = app.getWriter();

        String sref = "test:default -p 12010";
        com.zeroc.Ice.ObjectPrx obj = communicator.stringToProxy(sref);
        test(obj != null);

        TestIntfPrx test = TestIntfPrx.checkedCast(obj);
        test(test != null);

        out.print("testing connection... ");
        out.flush();
        {
            test.ice_ping();
        }
        out.println("ok");

        out.print("testing connection information... ");
        out.flush();
        {
            com.zeroc.Ice.IPConnectionInfo info = null;
            for(com.zeroc.Ice.ConnectionInfo p = test.ice_getConnection().getInfo(); p != null; p = p.underlying)
            {
                if(p instanceof com.zeroc.Ice.IPConnectionInfo)
                {
                    info = (com.zeroc.Ice.IPConnectionInfo)p;
                }
            }
            test(info.remotePort == 12030 || info.remotePort == 12031); // make sure we are connected to the proxy port.
        }
        out.println("ok");

        out.print("shutting down server... ");
        out.flush();
        {
            test.shutdown();
        }
        out.println("ok");

        out.print("testing connection failure... ");
        out.flush();
        {
            try
            {
                test.ice_ping();
                test(false);
            }
            catch(com.zeroc.Ice.LocalException ex)
            {
            }
        }
        out.println("ok");
    }
}
