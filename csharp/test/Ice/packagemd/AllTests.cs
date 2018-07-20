// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using Test;

public class AllTests : Test.AllTests
{
    public static InitialPrx allTests(TestHelper helper)
    {
        var communicator = helper.communicator();
        Console.Out.Write("testing stringToProxy... ");
        Console.Out.Flush();
        var @base = communicator.stringToProxy("initial:" + helper.getTestEndpoint(0));
        test(@base != null);
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing checked cast... ");
        Console.Out.Flush();
        var initial = InitialPrxHelper.checkedCast(@base);
        test(initial != null);
        test(initial.Equals(@base));
        Console.Out.WriteLine("ok");

        {
            Console.Out.Write("testing types without package... ");
            Console.Out.Flush();
            test.Ice.packagemd.Test1.C1 c1 = initial.getTest1C2AsC1();
            test(c1 != null);
            test(c1 is test.Ice.packagemd.Test1.C2);
            test.Ice.packagemd.Test1.C2 c2 = initial.getTest1C2AsC2();
            test(c2 != null);
            try
            {
                initial.throwTest1E2AsE1();
                test(false);
            }
            catch(test.Ice.packagemd.Test1.E1 ex)
            {
                test(ex is test.Ice.packagemd.Test1.E2);
            }
            try
            {
                initial.throwTest1E2AsE2();
                test(false);
            }
            catch(test.Ice.packagemd.Test1.E2)
            {
                // Expected
            }
            try
            {
                initial.throwTest1Notify();
                test(false);
            }
            catch(test.Ice.packagemd.Test1.@notify)
            {
                // Expected
            }
            Console.Out.WriteLine("ok");
        }

        {
            Console.Out.Write("testing types with package... ");
            Console.Out.Flush();

            {
                try
                {
                    initial.throwTest2E2AsE1();
                    test(false);
                }
                catch(Ice.UnknownUserException)
                {
                    // Expected
                }
                catch(Ice.MarshalException)
                {
                    // Expected
                }
                catch(test.Ice.packagemd.testpkg.Test2.E1)
                {
                    test(false);
                }
                try
                {
                    initial.throwTest2E2AsE2();
                    test(false);
                }
                catch(Ice.UnknownUserException)
                {
                    // Expected
                }
                catch(Ice.MarshalException)
                {
                    // Expected
                }
                catch(test.Ice.packagemd.testpkg.Test2.E1)
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
                test(c1 is test.Ice.packagemd.testpkg.Test2.C2);
                test.Ice.packagemd.testpkg.Test2.C2 c2 = initial.getTest2C2AsC2();
                test(c2 != null);
                try
                {
                    initial.throwTest2E2AsE1();
                    test(false);
                }
                catch (test.Ice.packagemd.testpkg.Test2.E1 ex)
                {
                    test(ex is test.Ice.packagemd.testpkg.Test2.E2);
                }
                try
                {
                    initial.throwTest2E2AsE2();
                    test(false);
                }
                catch(test.Ice.packagemd.testpkg.Test2.E2)
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
                test(c1 is test.Ice.packagemd.modpkg.Test3.C2);
                test.Ice.packagemd.modpkg.Test3.C2 c2 = initial.getTest3C2AsC2();
                test(c2 != null);
                try
                {
                    initial.throwTest3E2AsE1();
                    test(false);
                }
                catch (test.Ice.packagemd.modpkg.Test3.E1 ex)
                {
                    test(ex is test.Ice.packagemd.modpkg.Test3.E2);
                }
                try
                {
                    initial.throwTest3E2AsE2();
                    test(false);
                }
                catch (test.Ice.packagemd.modpkg.Test3.E2)
                {
                    // Expected
                }
            }

            Console.Out.WriteLine("ok");
        }
        return initial;
    }
}
