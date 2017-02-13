// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.ami;

import java.io.PrintWriter;

import test.Ice.ami.Test.TestIntfPrx;
import test.Ice.ami.Test.TestIntfControllerPrx;
import test.Util.Application;

public class AllTests
{
    private static void test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    public static void allTests(Application app, boolean collocated)
    {
        com.zeroc.Ice.Communicator communicator = app.communicator();
        PrintWriter out = app.getWriter();

        String sref = "test:" + app.getTestEndpoint(0);
        com.zeroc.Ice.ObjectPrx obj = communicator.stringToProxy(sref);
        test(obj != null);

        TestIntfPrx p = TestIntfPrx.uncheckedCast(obj);

        sref = "testController:" + app.getTestEndpoint(1);
        obj = communicator.stringToProxy(sref);
        test(obj != null);

        TestIntfControllerPrx testController = TestIntfControllerPrx.uncheckedCast(obj);

        test.Ice.ami.AMI.run(app, communicator, collocated, p, testController);

        p.shutdown();
    }
}
