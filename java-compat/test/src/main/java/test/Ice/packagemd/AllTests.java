// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.packagemd;

import java.io.PrintWriter;

import test.Ice.packagemd.Test.InitialPrx;
import test.Ice.packagemd.Test.InitialPrxHelper;
import test.Ice.packagemd.Test1.C1;
import test.Ice.packagemd.Test1.C2;
import test.Ice.packagemd.Test1.E1;
import test.Ice.packagemd.Test1.E2;
import test.Ice.packagemd.Test1._notify;

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

    public static InitialPrx
    allTests(test.TestHelper helper)
    {
        Ice.Communicator communicator = helper.communicator();
        PrintWriter out = helper.getWriter();
                out.print("testing stringToProxy... ");
        out.flush();
        String ref = "initial:" + helper.getTestEndpoint(0);
        Ice.ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);
        out.println("ok");

        out.print("testing checked cast... ");
        out.flush();
        InitialPrx initial = InitialPrxHelper.checkedCast(base);
        test(initial != null);
        test(initial.equals(base));
        out.println("ok");

        {
            out.print("testing types without package... ");
            out.flush();
            C1 c1 = initial.getTest1C2AsC1();
            test(c1 != null);
            test(c1 instanceof C2);
            C2 c2 = initial.getTest1C2AsC2();
            test(c2 != null);
            try
            {
                initial.throwTest1E2AsE1();
                test(false);
            }
            catch(E1 ex)
            {
                test(ex instanceof E2);
            }
            try
            {
                initial.throwTest1E2AsE2();
                test(false);
            }
            catch(E2 ex)
            {
                // Expected
            }
            try
            {
                initial.throwTest1Notify();
                test(false);
            }
            catch(_notify ex)
            {
                // Expected
            }
            out.println("ok");
        }

        {
            out.print("testing types with package... ");
            out.flush();

            {
                try
                {
                    initial.throwTest2E2AsE1();
                    test(false);
                }
                catch(Ice.UnknownUserException ex)
                {
                    // Expected
                }
                catch(Ice.MarshalException ex)
                {
                    // Expected
                }
                catch(test.Ice.packagemd.testpkg.Test2.E1 ex)
                {
                    test(false);
                }
                try
                {
                    initial.throwTest2E2AsE2();
                    test(false);
                }
                catch(Ice.UnknownUserException ex)
                {
                    // Expected
                }
                catch(Ice.MarshalException ex)
                {
                    // Expected
                }
                catch(test.Ice.packagemd.testpkg.Test2.E1 ex)
                {
                    test(false);
                }
            }

            {
                //
                // Define Ice.Package.Test2=testpkg and try again.
                //
                communicator.getProperties().setProperty("Ice.Package.Test2", "test.Ice.packagemd.testpkg");
                test.Ice.packagemd.testpkg.Test2.C1 c1 = initial.getTest2C2AsC1();
                test(c1 != null);
                test(c1 instanceof test.Ice.packagemd.testpkg.Test2.C2);
                test.Ice.packagemd.testpkg.Test2.C2 c2 = initial.getTest2C2AsC2();
                test(c2 != null);
                try
                {
                    initial.throwTest2E2AsE1();
                    test(false);
                }
                catch(test.Ice.packagemd.testpkg.Test2.E1 ex)
                {
                    test(ex instanceof test.Ice.packagemd.testpkg.Test2.E2);
                }
                try
                {
                    initial.throwTest2E2AsE2();
                    test(false);
                }
                catch(test.Ice.packagemd.testpkg.Test2.E2 ex)
                {
                    // Expected
                }
            }

            {
                //
                // Define Ice.Default.Package=testpkg and try again. We can't retrieve
                // the Test2.* types again (with this communicator) because factories
                // have already been cached for them, so now we use the Test3.* types.
                //
                communicator.getProperties().setProperty("Ice.Default.Package", "test.Ice.packagemd.modpkg");
                test.Ice.packagemd.modpkg.Test3.C1 c1 = initial.getTest3C2AsC1();
                test(c1 != null);
                test(c1 instanceof test.Ice.packagemd.modpkg.Test3.C2);
                test.Ice.packagemd.modpkg.Test3.C2 c2 = initial.getTest3C2AsC2();
                test(c2 != null);
                try
                {
                    initial.throwTest3E2AsE1();
                    test(false);
                }
                catch(test.Ice.packagemd.modpkg.Test3.E1 ex)
                {
                    test(ex instanceof test.Ice.packagemd.modpkg.Test3.E2);
                }
                try
                {
                    initial.throwTest3E2AsE2();
                    test(false);
                }
                catch(test.Ice.packagemd.modpkg.Test3.E2 ex)
                {
                    // Expected
                }
            }

            out.println("ok");
        }

        return initial;
    }
}
