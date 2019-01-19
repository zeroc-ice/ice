//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.operations;

import java.io.PrintWriter;

import test.Ice.operations.Test.MyClassPrx;
import test.Ice.operations.Test.MyClassPrxHelper;
import test.Ice.operations.Test.MyDerivedClassPrx;
import test.Ice.operations.Test.MyDerivedClassPrxHelper;

public class AllTests
{
    public static MyClassPrx
    allTests(test.TestHelper helper)
    {
        Ice.Communicator communicator = helper.communicator();
        PrintWriter out = helper.getWriter();

        String ref = "test:" + helper.getTestEndpoint(0);
        Ice.ObjectPrx base = communicator.stringToProxy(ref);
        MyClassPrx cl = MyClassPrxHelper.checkedCast(base);
        MyDerivedClassPrx derived = MyDerivedClassPrxHelper.checkedCast(cl);

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

        //
        // Use reflection to load TwowaysLambdaAMI as that is only supported with Java >= 1.8
        //
        try
        {
            Class<?> cls = IceInternal.Util.findClass("test.Ice.operations.lambda.TwowaysLambdaAMI", null);
            if(cls != null)
            {
                java.lang.reflect.Method twowaysLambdaAMI =
                    cls.getDeclaredMethod("twowaysLambdaAMI",
                                          new Class<?>[]{test.TestHelper.class, MyClassPrx.class});
                out.print("testing twoway operations with lambda AMI mapping... ");
                out.flush();
                twowaysLambdaAMI.invoke(null, helper, cl);
                twowaysLambdaAMI.invoke(null, helper, derived);
                out.println("ok");
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

        out.print("testing oneway operations with AMI... ");
        out.flush();
        OnewaysAMI.onewaysAMI(helper, cl);
        out.println("ok");

        //
        // Use reflection to load OnewaysLambdaAMI as that is only supported with Java >= 1.8
        //
        try
        {
            Class<?> cls = IceInternal.Util.findClass("test.Ice.operations.lambda.OnewaysLambdaAMI", null);
            if(cls != null)
            {
                java.lang.reflect.Method onewaysLambdaAMI =
                    cls.getDeclaredMethod("onewaysLambdaAMI",
                                          new Class<?>[]{test.TestHelper.class, MyClassPrx.class});
                out.print("testing twoway operations with lambda AMI mapping... ");
                out.flush();
                onewaysLambdaAMI.invoke(null, helper, cl);
                onewaysLambdaAMI.invoke(null, helper, derived);
                out.println("ok");
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
