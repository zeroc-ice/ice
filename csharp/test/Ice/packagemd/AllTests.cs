// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Test;

namespace Ice
{
    namespace packagemd
    {
        public class AllTests : global::Test.AllTests
        {
            public static Test.InitialPrx allTests(TestHelper helper)
            {
                var communicator = helper.communicator();
                var output = helper.getWriter();
                output.Write("testing stringToProxy... ");
                output.Flush();
                var @base = communicator.stringToProxy("initial:" + helper.getTestEndpoint(0));
                test(@base != null);
                output.WriteLine("ok");

                output.Write("testing checked cast... ");
                output.Flush();
                var initial = Test.InitialPrxHelper.checkedCast(@base);
                test(initial != null);
                test(initial.Equals(@base));
                output.WriteLine("ok");

                {
                    output.Write("testing types without package... ");
                    output.Flush();
                    Test1.C1 c1 = initial.getTest1C2AsC1();
                    test(c1 != null);
                    test(c1 is Test1.C2);
                    Test1.C2 c2 = initial.getTest1C2AsC2();
                    test(c2 != null);
                    try
                    {
                        initial.throwTest1E2AsE1();
                        test(false);
                    }
                    catch(Test1.E1 ex)
                    {
                        test(ex is Test1.E2);
                    }
                    try
                    {
                        initial.throwTest1E2AsE2();
                        test(false);
                    }
                    catch(Test1.E2)
                    {
                        // Expected
                    }
                    try
                    {
                        initial.throwTest1Notify();
                        test(false);
                    }
                    catch(Test1.@notify)
                    {
                        // Expected
                    }
                    output.WriteLine("ok");
                }

                {
                    output.Write("testing types with package... ");
                    output.Flush();

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
                        catch(Ice.packagemd.testpkg.Test2.E1)
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
                        catch(testpkg.Test2.E1)
                        {
                            test(false);
                        }
                    }

                    {
                        //
                        // Define Ice.Package.Test2=testpkg and try again.
                        //
                        communicator.getProperties().setProperty("Ice.Package.Test2", "Ice.packagemd.testpkg");
                        testpkg.Test2.C1 c1 = initial.getTest2C2AsC1();
                        test(c1 != null);
                        test(c1 is testpkg.Test2.C2);
                        testpkg.Test2.C2 c2 = initial.getTest2C2AsC2();
                        test(c2 != null);
                        try
                        {
                            initial.throwTest2E2AsE1();
                            test(false);
                        }
                        catch(testpkg.Test2.E1 ex)
                        {
                            test(ex is testpkg.Test2.E2);
                        }
                        try
                        {
                            initial.throwTest2E2AsE2();
                            test(false);
                        }
                        catch(testpkg.Test2.E2)
                        {
                            // Expected
                        }
                    }

                    {
                        //
                        // Define Ice.Default.Package=testpkg and try again. We can't retrieve
                        // the Test2.* types again(with this communicator) because factories
                        // have already been cached for them, so now we use the Test3.* types.
                        //
                        communicator.getProperties().setProperty("Ice.Default.Package", "Ice.packagemd.modpkg");
                        modpkg.Test3.C1 c1 = initial.getTest3C2AsC1();
                        test(c1 != null);
                        test(c1 is modpkg.Test3.C2);
                        modpkg.Test3.C2 c2 = initial.getTest3C2AsC2();
                        test(c2 != null);
                        try
                        {
                            initial.throwTest3E2AsE1();
                            test(false);
                        }
                        catch(modpkg.Test3.E1 ex)
                        {
                            test(ex is modpkg.Test3.E2);
                        }
                        try
                        {
                            initial.throwTest3E2AsE2();
                            test(false);
                        }
                        catch(modpkg.Test3.E2)
                        {
                            // Expected
                        }
                    }

                    output.WriteLine("ok");
                }
                return initial;
            }
        }
    }
}
