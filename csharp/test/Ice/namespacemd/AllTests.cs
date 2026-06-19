// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice.namespacemd;

public class AllTests : global::Test.AllTests
{
    public static Test.InitialPrx allTests(TestHelper helper)
    {
        Communicator communicator = helper.communicator();
        TextWriter output = helper.getWriter();
        output.Write("testing stringToProxy... ");
        output.Flush();
        ObjectPrx @base = communicator.stringToProxy("initial:" + helper.getTestEndpoint(0));
        test(@base != null);
        output.WriteLine("ok");

        output.Write("testing checked cast... ");
        output.Flush();
        Test.InitialPrx initial = Test.InitialPrxHelper.checkedCast(@base);
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
            catch (NoNamespace.E1 ex)
            {
                test(ex is NoNamespace.E2);
            }
            try
            {
                initial.throwNoNamespaceE2AsE2();
                test(false);
            }
            catch (NoNamespace.E2)
            {
                // Expected
            }
            try
            {
                initial.throwNoNamespaceNotify();
                test(false);
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
                // A type in a module nested under a cs:namespace-tagged module must resolve at
                // WithNamespace.Inner.S (single prefix), not a doubled prefix (regression test for #5478).
                test(typeof(WithNamespace.Inner.S).Namespace == "Ice.namespacemd.WithNamespace.Inner");
                test(typeof(global::ZeroC.Foo.Bar.S).Namespace == "ZeroC.Foo.Bar");
                test(typeof(global::ZeroC.Other.Bar.S).Namespace == "ZeroC.Other.Bar");
                test(typeof(global::ZeroC.Other.Baz.S).Namespace == "ZeroC.Other.Baz");

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
                catch (WithNamespace.E1 ex)
                {
                    test(ex is WithNamespace.E2);
                }
                try
                {
                    initial.throwWithNamespaceE2AsE2();
                    test(false);
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
