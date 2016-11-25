// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.operations;

import java.io.PrintWriter;

import test.Ice.operations.Test.MyClassPrx;
import test.Ice.operations.Test.MyDerivedClassPrx;

public class AllTests
{
    public static MyClassPrx allTests(test.Util.Application app)
    {
        PrintWriter out = app.getWriter();
        com.zeroc.Ice.Communicator communicator = app.communicator();
        String ref = "test:" + app.getTestEndpoint(0);
        com.zeroc.Ice.ObjectPrx base = communicator.stringToProxy(ref);
        MyClassPrx cl = MyClassPrx.checkedCast(base);
        MyDerivedClassPrx derived = MyDerivedClassPrx.checkedCast(cl);

        out.print("testing twoway operations... ");
        out.flush();
        Twoways.twoways(app, cl);
        Twoways.twoways(app, derived);
        derived.opDerived();
        out.println("ok");

        out.print("testing oneway operations... ");
        out.flush();
        Oneways.oneways(app, cl);
        out.println("ok");

        out.print("testing twoway operations with AMI... ");
        out.flush();
        TwowaysAMI.twowaysAMI(app, cl);
        TwowaysAMI.twowaysAMI(app, derived);
        out.println("ok");

        out.print("testing oneway operations with AMI... ");
        out.flush();
        OnewaysAMI.onewaysAMI(app, cl);
        out.println("ok");

        out.print("testing batch oneway operations... ");
        out.flush();
        BatchOneways.batchOneways(app, cl, out);
        BatchOneways.batchOneways(app, derived, out);
        out.println("ok");

        out.print("testing batch AMI oneway operations... ");
        out.flush();
        BatchOnewaysAMI.batchOneways(cl, out);
        BatchOnewaysAMI.batchOneways(derived, out);
        out.println("ok");
        return cl;
    }
}
