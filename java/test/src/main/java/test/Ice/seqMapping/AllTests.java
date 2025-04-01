// Copyright (c) ZeroC, Inc.

package test.Ice.seqMapping;

import test.Ice.seqMapping.Test.*;

import java.io.PrintWriter;

public class AllTests {
    public static MyClassPrx allTests(test.TestHelper helper, boolean collocated) {
        PrintWriter out = helper.getWriter();
        com.zeroc.Ice.Communicator communicator = helper.communicator();

        String ref = "test:" + helper.getTestEndpoint(0);
        com.zeroc.Ice.ObjectPrx baseProxy = communicator.stringToProxy(ref);
        MyClassPrx cl = MyClassPrx.checkedCast(baseProxy);

        out.print("testing twoway operations... ");
        out.flush();
        Twoways.twoways(cl);
        out.println("ok");

        if (!collocated) {
            out.print("testing twoway operations with AMI... ");
            out.flush();
            TwowaysAMI.twowaysAMI(cl);
            out.println("ok");
        }

        return cl;
    }

    private AllTests() {
    }
}
