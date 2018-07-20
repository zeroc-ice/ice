// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.seqMapping;

import java.io.PrintWriter;

import test.Ice.seqMapping.Test.*;

public class AllTests
{
    public static MyClassPrx
    allTests(test.TestHelper helper, boolean collocated)
    {
        Ice.Communicator communicator = helper.communicator();
        PrintWriter out = helper.getWriter();

        String ref = "test:" + helper.getTestEndpoint(0);
        Ice.ObjectPrx baseProxy = communicator.stringToProxy(ref);
        MyClassPrx cl = MyClassPrxHelper.checkedCast(baseProxy);

        out.print("testing twoway operations... ");
        out.flush();
        Twoways.twoways(cl);
        out.println("ok");

        if(!collocated)
        {
            out.print("testing twoway operations with AMI... ");
            out.flush();
            TwowaysAMI.twowaysAMI(cl);
            out.println("ok");
        }

        return cl;
    }
}
