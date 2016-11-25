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
import test.Ice.operations.Test.MyClassPrxHelper;
import test.Ice.operations.Test.MyDerivedClassPrx;
import test.Ice.operations.Test.MyDerivedClassPrxHelper;

public class AllTests
{
    public static MyClassPrx
    allTests(test.Util.Application app)
    {
        Ice.Communicator communicator = app.communicator();
        PrintWriter out = app.getWriter();

        String ref = "test:" + app.getTestEndpoint(0);
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

        out.print("testing twoway operations with AMI... ");
        out.flush();
        TwowaysAMI.twowaysAMI(app, cl);
        TwowaysAMI.twowaysAMI(app, derived);
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
                                          new Class<?>[]{test.Util.Application.class, MyClassPrx.class});
                out.print("testing twoway operations with lambda AMI mapping... ");
                out.flush();
                twowaysLambdaAMI.invoke(null, app, cl);
                twowaysLambdaAMI.invoke(null, app, derived);
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
        OnewaysAMI.onewaysAMI(app, cl);
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
                                          new Class<?>[]{test.Util.Application.class, MyClassPrx.class});
                out.print("testing twoway operations with lambda AMI mapping... ");
                out.flush();
                onewaysLambdaAMI.invoke(null, app, cl);
                onewaysLambdaAMI.invoke(null, app, derived);
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
