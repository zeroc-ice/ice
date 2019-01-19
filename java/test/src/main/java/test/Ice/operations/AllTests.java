//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.operations;

import java.io.PrintWriter;

import test.Ice.operations.Test.MyClassPrx;
import test.Ice.operations.Test.MyDerivedClassPrx;

public class AllTests
{
    public static MyClassPrx allTests(test.TestHelper helper)
    {
        PrintWriter out = helper.getWriter();
        com.zeroc.Ice.Communicator communicator = helper.communicator();
        String ref = "test:" + helper.getTestEndpoint(0);
        com.zeroc.Ice.ObjectPrx base = communicator.stringToProxy(ref);
        MyClassPrx cl = MyClassPrx.checkedCast(base);
        MyDerivedClassPrx derived = MyDerivedClassPrx.checkedCast(cl);

        out.print("testing twoway operations... ");
        out.flush();
        Twoways.twoways(helper, cl);
        Twoways.twoways(helper, derived);
        derived.opDerived();
        out.println("ok");

        out.print("testing oneway operations... ");
        out.flush();
        Oneways.oneways(helper, cl);
        out.println("ok");

        out.print("testing twoway operations with AMI... ");
        out.flush();
        TwowaysAMI.twowaysAMI(helper, cl);
        TwowaysAMI.twowaysAMI(helper, derived);
        out.println("ok");

        out.print("testing oneway operations with AMI... ");
        out.flush();
        OnewaysAMI.onewaysAMI(helper, cl);
        out.println("ok");

        out.print("testing batch oneway operations... ");
        out.flush();
        BatchOneways.batchOneways(helper, cl, out);
        BatchOneways.batchOneways(helper, derived, out);
        out.println("ok");

        out.print("testing batch AMI oneway operations... ");
        out.flush();
        BatchOnewaysAMI.batchOneways(cl, out);
        BatchOnewaysAMI.batchOneways(derived, out);
        out.println("ok");
        return cl;
    }
}
