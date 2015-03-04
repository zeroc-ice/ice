// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.operations;
import java.io.PrintWriter;

import test.Ice.operations.Test.MyClassPrx;
import test.Ice.operations.Test.MyClassPrxHelper;
import test.Ice.operations.Test.MyDerivedClassPrx;
import test.Ice.operations.Test.MyDerivedClassPrxHelper;

public class AllTests
{
    public static MyClassPrx
    allTests(test.Util.Application app, boolean collocated, PrintWriter out)
    {
        Ice.Communicator communicator = app.communicator();
        String ref = "test:default -p 12010";
        Ice.ObjectPrx base = communicator.stringToProxy(ref);
        MyClassPrx cl = MyClassPrxHelper.checkedCast(base);
        MyDerivedClassPrx derived = MyDerivedClassPrxHelper.checkedCast(cl);

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

        if(!collocated)
        {
            out.print("testing twoway operations with AMI... ");
            out.flush();
            TwowaysAMI.twowaysAMI(app, cl);
            TwowaysAMI.twowaysAMI(app, derived);
            out.println("ok");

            out.print("testing twoway operations with new AMI mapping... ");
            out.flush();
            TwowaysNewAMI.twowaysNewAMI(app, cl);
            TwowaysNewAMI.twowaysNewAMI(app, derived);
            out.println("ok");

            out.print("testing oneway operations with AMI... ");
            out.flush();
            OnewaysAMI.onewaysAMI(app, cl);
            out.println("ok");

            out.print("testing oneway operations with new AMI mapping... ");
            out.flush();
            OnewaysNewAMI.onewaysNewAMI(app, cl);
            out.println("ok");

            out.print("testing batch oneway operations... ");
            out.flush();
            BatchOneways.batchOneways(cl, out);
            BatchOneways.batchOneways(derived, out);
            out.println("ok");
        }

        return cl;
    }
}
