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
    namespace namespacemd
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
                    NoNamespace.C1 c1 = initial.getNoNamespaceC2AsC1();
                    test(c1 != null);
                    test(c1 is NoNamespace.C2);
                    NoNamespace.C2 c2 = initial.getNoNamespaceC2AsC2();
                    test(c2 != null);
                    try
                    {
                        initial.throwNoNamespaceE2AsE1();
                        test(false);
                    }
                    catch(NoNamespace.E1 ex)
                    {
                        test(ex is NoNamespace.E2);
                    }
                    try
                    {
                        initial.throwNoNamespaceE2AsE2();
                        test(false);
                    }
                    catch(NoNamespace.E2)
                    {
                        // Expected
                    }
                    try
                    {
                        initial.throwNoNamespaceNotify();
                        test(false);
                    }
                    catch(NoNamespace.@notify)
                    {
                        // Expected
                    }
                    output.WriteLine("ok");
                }

                {
                    output.Write("testing types with package... ");
                    output.Flush();

                    {
                        WithNamespace.C1 c1 = initial.getWithNamespaceC2AsC1();
                        test(c1 != null);
                        test(c1 is WithNamespace.C2);
                        WithNamespace.C2 c2 = initial.getWithNamespaceC2AsC2();
                        test(c2 != null);
                        try
                        {
                            initial.throwWithNamespaceE2AsE1();
                            test(false);
                        }
                        catch(WithNamespace.E1 ex)
                        {
                            test(ex is WithNamespace.E2);
                        }
                        try
                        {
                            initial.throwWithNamespaceE2AsE2();
                            test(false);
                        }
                        catch(WithNamespace.E2)
                        {
                            // Expected
                        }
                        output.WriteLine("ok");
                    }
                }
                return initial;
            }
        }
    }
}
