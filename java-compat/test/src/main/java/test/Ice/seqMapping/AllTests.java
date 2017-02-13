// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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
    allTests(test.Util.Application app, boolean collocated)
    {
        Ice.Communicator communicator = app.communicator();
        PrintWriter out = app.getWriter();

        String ref = "test:" + app.getTestEndpoint(0);
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
