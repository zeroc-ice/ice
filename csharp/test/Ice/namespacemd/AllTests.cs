//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace Ice.namespacemd
{
    public class AllTests
    {
        public static Test.IInitialPrx allTests(TestHelper helper)
        {
            Communicator? communicator = helper.Communicator();
            TestHelper.Assert(communicator != null);
            System.IO.TextWriter output = helper.GetWriter();
            output.Write("testing stringToProxy... ");
            output.Flush();
            var @base = IObjectPrx.Parse($"initial:{helper.GetTestEndpoint(0)}", communicator);
            output.WriteLine("ok");

            output.Write("testing checked cast... ");
            output.Flush();
            var initial = Test.IInitialPrx.CheckedCast(@base);
            TestHelper.Assert(initial != null);
            TestHelper.Assert(initial.Equals(@base));
            output.WriteLine("ok");

            {
                output.Write("testing types without package... ");
                output.Flush();
                NoNamespace.C1? c1 = initial.getNoNamespaceC2AsC1();
                TestHelper.Assert(c1 != null);
                TestHelper.Assert(c1 is NoNamespace.C2);
                NoNamespace.C2? c2 = initial.getNoNamespaceC2AsC2();
                TestHelper.Assert(c2 != null);
                try
                {
                    initial.throwNoNamespaceE2AsE1();
                    TestHelper.Assert(false);
                }
                catch (NoNamespace.E1 ex)
                {
                    TestHelper.Assert(ex is NoNamespace.E2);
                }
                try
                {
                    initial.throwNoNamespaceE2AsE2();
                    TestHelper.Assert(false);
                }
                catch (NoNamespace.E2)
                {
                    // Expected
                }
                try
                {
                    initial.throwNoNamespaceNotify();
                    TestHelper.Assert(false);
                }
                catch (NoNamespace.@notify)
                {
                    // Expected
                }
                output.WriteLine("ok");
            }

            {
                output.Write("testing types with package... ");
                output.Flush();

                {
                    WithNamespace.C1? c1 = initial.getWithNamespaceC2AsC1();
                    TestHelper.Assert(c1 != null);
                    TestHelper.Assert(c1 is WithNamespace.C2);
                    WithNamespace.C2? c2 = initial.getWithNamespaceC2AsC2();
                    TestHelper.Assert(c2 != null);
                    try
                    {
                        initial.throwWithNamespaceE2AsE1();
                        TestHelper.Assert(false);
                    }
                    catch (WithNamespace.E1 ex)
                    {
                        TestHelper.Assert(ex is WithNamespace.E2);
                    }
                    try
                    {
                        initial.throwWithNamespaceE2AsE2();
                        TestHelper.Assert(false);
                    }
                    catch (WithNamespace.E2)
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
