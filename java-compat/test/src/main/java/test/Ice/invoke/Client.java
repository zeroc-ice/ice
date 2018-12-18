// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.invoke;

import test.Ice.invoke.Test.MyClassPrx;

public class Client extends test.TestHelper
{
    public void run(String[] args)
    {
        Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.invoke");
        try(Ice.Communicator communicator = initialize(properties))
        {
            MyClassPrx myClass = AllTests.allTests(this);

            //
            // Use reflection to load lambda.AllTests as that is only supported with Java >= 1.8
            //
            try
            {
                Class<?> cls = IceInternal.Util.findClass("test.Ice.invoke.lambda.AllTests", null);
                if(cls != null)
                {
                    java.lang.reflect.Method allTests =
                        cls.getDeclaredMethod("allTests", new Class<?>[]{ test.TestHelper.class });
                    allTests.invoke(null, this);
                }
            }
            catch(Exception ex)
            {
                throw new RuntimeException(ex);
            }
            myClass.shutdown();
        }
    }
}
