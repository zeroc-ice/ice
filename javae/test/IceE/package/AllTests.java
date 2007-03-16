// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

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

    public static Test.InitialPrx
    allTests(Ice.Communicator communicator, java.io.PrintStream out)
    {
        out.print("testing stringToProxy... ");
        out.flush();
        String ref = communicator.getProperties().getPropertyWithDefault("Test.Proxy", 
		"initial:default -p 12010 -t 10000");
        Ice.ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);
        out.println("ok");

        out.print("testing checked cast... ");
        out.flush();
        Test.InitialPrx initial = Test.InitialPrxHelper.checkedCast(base);
        test(initial != null);
        test(initial.equals(base));
        out.println("ok");

        {
            out.print("testing types without package... ");
            out.flush();
            try
            {
                initial.throwTest1E2AsE1();
                test(false);
            }
            catch(Test1.E1 ex)
            {
                test(ex instanceof Test1.E2);
            }
            try
            {
                initial.throwTest1E2AsE2();
                test(false);
            }
            catch(Test1.E2 ex)
            {
                // Expected
            }
            try
            {
                initial.throwTest1Notify();
                test(false);
            }
            catch(Test1._notify ex)
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
                catch(Ice.MarshalException ex)
                {
                    // Expected
                }
                catch(testpkg.Test2.E1 ex)
                {
                    test(false);
                }
                try
                {
                    initial.throwTest2E2AsE2();
                    test(false);
                }
                catch(Ice.MarshalException ex)
                {
                    // Expected
                }
                catch(testpkg.Test2.E1 ex)
                {
                    test(false);
                }
            }

            {
                //
                // Define Ice.Package.Test2=testpkg and try again.
                //
                communicator.getProperties().setProperty("Ice.Package.Test2", "testpkg");
                try
                {
                    initial.throwTest2E2AsE1();
                    test(false);
                }
                catch(testpkg.Test2.E1 ex)
                {
                    test(ex instanceof testpkg.Test2.E2);
                }
                try
                {
                    initial.throwTest2E2AsE2();
                    test(false);
                }
                catch(testpkg.Test2.E2 ex)
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
                communicator.getProperties().setProperty("Ice.Default.Package", "testpkg");
                try
                {
                    initial.throwTest3E2AsE1();
                    test(false);
                }
                catch(testpkg.Test3.E1 ex)
                {
                    test(ex instanceof testpkg.Test3.E2);
                }
                try
                {
                    initial.throwTest3E2AsE2();
                    test(false);
                }
                catch(testpkg.Test3.E2 ex)
                {
                    // Expected
                }
            }

            out.println("ok");
        }

        return initial;
    }
}
