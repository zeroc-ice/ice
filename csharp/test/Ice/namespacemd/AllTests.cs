// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.NamespaceMD
{
    public static class AllTests
    {
        public static async Task RunAsync(TestHelper helper)
        {
            Communicator communicator = helper.Communicator;

            System.IO.TextWriter output = helper.Output;
            output.Write("testing stringToProxy... ");
            output.Flush();
            var initial = IInitialPrx.Parse(helper.GetTestProxy("initial", 0), communicator);
            output.WriteLine("ok");

            {
                output.Write("testing types without package... ");
                output.Flush();
                NoNamespace.C1? c1 = initial.GetNoNamespaceC2AsC1();
                TestHelper.Assert(c1 != null);
                TestHelper.Assert(c1 is NoNamespace.C2);
                NoNamespace.C2? c2 = initial.GetNoNamespaceC2AsC2();
                TestHelper.Assert(c2 != null);
                try
                {
                    initial.ThrowNoNamespaceE2AsE1();
                    TestHelper.Assert(false);
                }
                catch (NoNamespace.E1 ex)
                {
                    TestHelper.Assert(ex is NoNamespace.E2);
                }
                try
                {
                    initial.ThrowNoNamespaceE2AsE2();
                    TestHelper.Assert(false);
                }
                catch (NoNamespace.E2)
                {
                    // Expected
                }
                try
                {
                    initial.ThrowNoNamespaceNotify();
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
                    WithNamespace.C1? c1 = initial.GetWithNamespaceC2AsC1();
                    TestHelper.Assert(c1 != null);
                    TestHelper.Assert(c1 is WithNamespace.C2);
                    WithNamespace.C2? c2 = initial.GetWithNamespaceC2AsC2();
                    TestHelper.Assert(c2 != null);
                    try
                    {
                        initial.ThrowWithNamespaceE2AsE1();
                        TestHelper.Assert(false);
                    }
                    catch (WithNamespace.E1 ex)
                    {
                        TestHelper.Assert(ex is WithNamespace.E2);
                    }
                    try
                    {
                        initial.ThrowWithNamespaceE2AsE2();
                        TestHelper.Assert(false);
                    }
                    catch (WithNamespace.E2)
                    {
                        // Expected
                    }
                    output.WriteLine("ok");
                }
            }
            await initial.ShutdownAsync();
        }
    }
}
