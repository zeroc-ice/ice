// Copyright (c) ZeroC, Inc.

package test.Ice.networkProxy;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.ConnectionInfo;
import com.zeroc.Ice.IPConnectionInfo;
import com.zeroc.Ice.LocalException;
import com.zeroc.Ice.ObjectPrx;

import test.Ice.networkProxy.Test.TestIntfPrx;
import test.TestHelper;

import java.io.PrintWriter;

public class AllTests {
    private static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    public static void allTests(TestHelper helper) {
        Communicator communicator = helper.communicator();
        PrintWriter out = helper.getWriter();

        String sref = "test:" + helper.getTestEndpoint(0);
        ObjectPrx obj = communicator.stringToProxy(sref);
        test(obj != null);

        int proxyPort = communicator.getProperties().getIcePropertyAsInt("Ice.HTTPProxyPort");
        // default, i.e. most likely not set
        if (proxyPort == 1080) {
            proxyPort = communicator.getProperties().getIcePropertyAsInt("Ice.SOCKSProxyPort");
        }

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
            IPConnectionInfo info = null;
            for (ConnectionInfo p = test.ice_getConnection().getInfo();
                    p != null;
                    p = p.underlying) {
                if (p instanceof IPConnectionInfo) {
                    info = (IPConnectionInfo) p;
                }
            }
            test(info.remotePort == proxyPort); // make sure we are connected to the proxy port.
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
            try {
                test.ice_ping();
                test(false);
            } catch (LocalException ex) {
            }
        }
        out.println("ok");
    }

    private AllTests() {
    }
}
