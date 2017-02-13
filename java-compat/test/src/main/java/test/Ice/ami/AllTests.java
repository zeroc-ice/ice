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
import test.Ice.ami.Test.TestIntfPrxHelper;
import test.Ice.ami.Test.TestIntfControllerPrx;
import test.Ice.ami.Test.TestIntfControllerPrxHelper;
import test.Util.Application;

public class AllTests
{
    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    public static void
    allTests(Application app, boolean collocated)
    {
        Ice.Communicator communicator = app.communicator();
        PrintWriter out = app.getWriter();

        String sref = "test:" + app.getTestEndpoint(0);
        Ice.ObjectPrx obj = communicator.stringToProxy(sref);
        test(obj != null);

        TestIntfPrx p = TestIntfPrxHelper.uncheckedCast(obj);

        sref = "testController:" + app.getTestEndpoint(1);
        obj = communicator.stringToProxy(sref);
        test(obj != null);

        TestIntfControllerPrx testController = TestIntfControllerPrxHelper.uncheckedCast(obj);

        out.println("testing with new AMI mapping... ");
        test.Ice.ami.AMI.run(app, communicator, collocated, p, testController);

        //
        // Use reflection to load TwowaysLambdaAMI as that is only supported with Java >= 1.8
        //
        try
        {
            Class<?> cls = IceInternal.Util.findClass("test.Ice.ami.lambda.AMI", null);
            if(cls != null)
            {
                java.lang.reflect.Method run = cls.getDeclaredMethod(
                    "run",
                    new Class<?>[]
                    {
                        test.Util.Application.class,
                        Ice.Communicator.class,
                        boolean.class,
                        TestIntfPrx.class,
                        TestIntfControllerPrx.class
                    });
                out.println("testing with lambda AMI mapping... ");
                out.flush();
                run.invoke(null, app, communicator, collocated, p, testController);
            }
        }
        catch(java.lang.NoSuchMethodException ex)
        {
            throw new RuntimeException(ex);
        }
        catch(java.lang.IllegalAccessException ex)
        {
            throw new RuntimeException(ex);
        }
        catch(java.lang.reflect.InvocationTargetException ex)
        {
            throw new RuntimeException(ex);
        }

        p.shutdown();
    }
}
