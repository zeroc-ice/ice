// Copyright (c) ZeroC, Inc.

package test.Ice.seqMapping;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.ObjectPrx;

import test.Ice.seqMapping.Test.MyClassPrx;
import test.TestHelper;

import java.io.PrintWriter;

public class AllTests {
    public static MyClassPrx allTests(TestHelper helper, boolean collocated) {
        PrintWriter out = helper.getWriter();
        Communicator communicator = helper.communicator();

        String ref = "test:" + helper.getTestEndpoint(0);
        ObjectPrx baseProxy = communicator.stringToProxy(ref);
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
