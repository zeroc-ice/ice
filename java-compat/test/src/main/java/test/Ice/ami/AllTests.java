// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.ami;

import java.io.PrintWriter;

import test.Ice.ami.Test.TestIntfPrx;
import test.Ice.ami.Test.TestIntfPrxHelper;
import test.Ice.ami.Test.TestIntfControllerPrx;
import test.Ice.ami.Test.TestIntfControllerPrxHelper;

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
    allTests(test.TestHelper helper, boolean collocated)
    {
        Ice.Communicator communicator = helper.communicator();
        PrintWriter out = helper.getWriter();

        String sref = "test:" + helper.getTestEndpoint(0);
        Ice.ObjectPrx obj = communicator.stringToProxy(sref);
        test(obj != null);

        TestIntfPrx p = TestIntfPrxHelper.uncheckedCast(obj);

        sref = "testController:" + helper.getTestEndpoint(1);
        obj = communicator.stringToProxy(sref);
        test(obj != null);

        TestIntfControllerPrx testController = TestIntfControllerPrxHelper.uncheckedCast(obj);

        out.println("testing with new AMI mapping... ");
        test.Ice.ami.AMI.run(helper, communicator, collocated, p, testController);

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
                        test.TestHelper.class,
                        Ice.Communicator.class,
                        boolean.class,
                        TestIntfPrx.class,
                        TestIntfControllerPrx.class
                    });
                out.println("testing with lambda AMI mapping... ");
                out.flush();
                run.invoke(null, helper, communicator, collocated, p, testController);
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
