// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice.scope;

public class AllTests : global::Test.AllTests
{
    public static void allTests(TestHelper helper)
    {
        Communicator communicator = helper.communicator();
        {
            Test.MyInterfacePrx i = Test.MyInterfacePrxHelper.createProxy(communicator, "i1:" + helper.getTestEndpoint());

            var s1 = new Test.MyStruct(0);
            Test.MyStruct s3 = i.opMyStruct(s1, out Test.MyStruct s2);
            test(s2.Equals(s1));
            test(s3.Equals(s1));

            var sseq1 = new Test.MyStruct[] { s1 };
            Test.MyStruct[] sseq3 = i.opMyStructSeq(sseq1, out Test.MyStruct[] sseq2);
            test(sseq2[0].Equals(s1));
            test(sseq3[0].Equals(s1));

            var smap1 = new Dictionary<string, Test.MyStruct>
            {
                ["a"] = s1
            };
            Dictionary<string, Test.MyStruct> smap3 =
                i.opMyStructMap(smap1, out Dictionary<string, Test.MyStruct> smap2);
            test(smap2["a"].Equals(s1));
            test(smap3["a"].Equals(s1));

            var c1 = new Test.MyClass(s1);
            Test.MyClass c3 = i.opMyClass(c1, out Test.MyClass c2);
            test(c2.s.Equals(c1.s));
            test(c3.s.Equals(c1.s));

            var cseq1 = new Test.MyClass[] { c1 };
            Test.MyClass[] cseq3 = i.opMyClassSeq(cseq1, out Test.MyClass[] cseq2);
            test(cseq2[0].s.Equals(s1));
            test(cseq3[0].s.Equals(s1));

            var cmap1 = new Dictionary<string, Test.MyClass>
            {
                ["a"] = c1
            };
            Dictionary<string, Test.MyClass> cmap3 = i.opMyClassMap(cmap1, out Dictionary<string, Test.MyClass> cmap2);
            test(cmap2["a"].s.Equals(s1));
            test(cmap3["a"].s.Equals(s1));

            Test.MyEnum e = i.opMyEnum(Test.MyEnum.v1);
            test(e == Test.MyEnum.v1);

            Test.MyOtherStruct s = i.opMyOtherStruct(new Test.MyOtherStruct("MyOtherStruct"));
            test(s.s == "MyOtherStruct");

            Test.MyOtherClass c = i.opMyOtherClass(new Test.MyOtherClass("MyOtherClass"));
            test(c.s == "MyOtherClass");
        }

        {
            Test.MyInterfacePrx i = Test.MyInterfacePrxHelper.createProxy(communicator, "i1:" + helper.getTestEndpoint());

            Task.Run(async () =>
                {
                    var s1 = new Test.MyStruct(0);
                    Test.MyInterface_OpMyStructResult opMyStructResult = await i.opMyStructAsync(s1);
                    test(s1.Equals(opMyStructResult.returnValue));
                    test(s1.Equals(opMyStructResult.s2));

                    var sseq1 = new Test.MyStruct[] { s1 };
                    Test.MyInterface_OpMyStructSeqResult opMyStructSeqResult = await i.opMyStructSeqAsync(sseq1);
                    test(opMyStructSeqResult.returnValue[0].Equals(s1));
                    test(opMyStructSeqResult.s2[0].Equals(s1));

                    var smap1 = new Dictionary<string, Test.MyStruct>
                    {
                        ["a"] = s1
                    };
                    Test.MyInterface_OpMyStructMapResult opMyStructMapResult = await i.opMyStructMapAsync(smap1);
                    test(opMyStructMapResult.returnValue["a"].Equals(s1));
                    test(opMyStructMapResult.s2["a"].Equals(s1));

                    var c1 = new Test.MyClass(s1);
                    Test.MyInterface_OpMyClassResult opMyClassResult = await i.opMyClassAsync(c1);
                    test(c1.s.Equals(opMyClassResult.returnValue.s));
                    test(c1.s.Equals(opMyClassResult.c2.s));

                    var cseq1 = new Test.MyClass[] { c1 };
                    Test.MyInterface_OpMyClassSeqResult opMyClassSeqResult = await i.opMyClassSeqAsync(cseq1);
                    test(opMyClassSeqResult.returnValue[0].s.Equals(s1));
                    test(opMyClassSeqResult.c2[0].s.Equals(s1));

                    var cmap1 = new Dictionary<string, Test.MyClass>
                    {
                        ["a"] = c1
                    };
                    Test.MyInterface_OpMyClassMapResult opMyClassMapResult = await i.opMyClassMapAsync(cmap1);
                    test(opMyClassMapResult.returnValue["a"].s.Equals(s1));
                    test(opMyClassMapResult.c2["a"].s.Equals(s1));

                    Test.MyEnum e = await i.opMyEnumAsync(Test.MyEnum.v1);
                    test(e == Test.MyEnum.v1);

                    Test.MyOtherStruct s = await i.opMyOtherStructAsync(new Test.MyOtherStruct("MyOtherStruct"));
                    test(s.s == "MyOtherStruct");

                    Test.MyOtherClass c = await i.opMyOtherClassAsync(new Test.MyOtherClass("MyOtherClass"));
                    test(c.s == "MyOtherClass");
                }).Wait();
        }

        {
            Test.Inner.MyInterfacePrx i =
                Test.Inner.MyInterfacePrxHelper.createProxy(communicator, "i2:" + helper.getTestEndpoint());

            var s1 = new Test.Inner.Inner2.MyStruct(0);
            Test.Inner.Inner2.MyStruct s3 = i.opMyStruct(s1, out Test.Inner.Inner2.MyStruct s2);
            test(s2.Equals(s1));
            test(s3.Equals(s1));

            var sseq1 = new Test.Inner.Inner2.MyStruct[] { s1 };
            Test.Inner.Inner2.MyStruct[] sseq3 = i.opMyStructSeq(sseq1, out Test.Inner.Inner2.MyStruct[] sseq2);
            test(sseq2[0].Equals(s1));
            test(sseq3[0].Equals(s1));

            var smap1 = new Dictionary<string, Test.Inner.Inner2.MyStruct>
            {
                ["a"] = s1
            };
            Dictionary<string, Test.Inner.Inner2.MyStruct> smap3 =
                i.opMyStructMap(smap1, out Dictionary<string, Test.Inner.Inner2.MyStruct> smap2);
            test(smap2["a"].Equals(s1));
            test(smap3["a"].Equals(s1));

            var c1 = new Test.Inner.Inner2.MyClass(s1);
            Test.Inner.Inner2.MyClass c3 = i.opMyClass(c1, out Test.Inner.Inner2.MyClass c2);
            test(c2.s.Equals(c1.s));
            test(c3.s.Equals(c1.s));

            var cseq1 = new Test.Inner.Inner2.MyClass[] { c1 };
            Test.Inner.Inner2.MyClass[] cseq3 = i.opMyClassSeq(cseq1, out Test.Inner.Inner2.MyClass[] cseq2);
            test(cseq2[0].s.Equals(s1));
            test(cseq3[0].s.Equals(s1));

            var cmap1 = new Dictionary<string, Test.Inner.Inner2.MyClass>
            {
                ["a"] = c1
            };
            Dictionary<string, Test.Inner.Inner2.MyClass> cmap3 =
                i.opMyClassMap(cmap1, out Dictionary<string, Test.Inner.Inner2.MyClass> cmap2);
            test(cmap2["a"].s.Equals(s1));
            test(cmap3["a"].s.Equals(s1));
        }

        {
            Test.Inner.MyInterfacePrx i =
                Test.Inner.MyInterfacePrxHelper.createProxy(communicator, "i2:" + helper.getTestEndpoint());

            Task.Run(async () =>
                {
                    var s1 = new Test.Inner.Inner2.MyStruct(0);
                    Test.Inner.MyInterface_OpMyStructResult opMyStructResult = await i.opMyStructAsync(s1);
                    test(s1.Equals(opMyStructResult.returnValue));
                    test(s1.Equals(opMyStructResult.s2));

                    var sseq1 = new Test.Inner.Inner2.MyStruct[] { s1 };
                    Test.Inner.MyInterface_OpMyStructSeqResult opMyStructSeqResult = await i.opMyStructSeqAsync(sseq1);
                    test(opMyStructSeqResult.returnValue[0].Equals(s1));
                    test(opMyStructSeqResult.s2[0].Equals(s1));

                    var smap1 = new Dictionary<string, Test.Inner.Inner2.MyStruct>
                    {
                        ["a"] = s1
                    };
                    Test.Inner.MyInterface_OpMyStructMapResult opMyStructMapResult = await i.opMyStructMapAsync(smap1);
                    test(opMyStructMapResult.returnValue["a"].Equals(s1));
                    test(opMyStructMapResult.s2["a"].Equals(s1));

                    var c1 = new Test.Inner.Inner2.MyClass(s1);
                    Test.Inner.MyInterface_OpMyClassResult opMyClassResult = await i.opMyClassAsync(c1);
                    test(c1.s.Equals(opMyClassResult.returnValue.s));
                    test(c1.s.Equals(opMyClassResult.c2.s));

                    var cseq1 = new Test.Inner.Inner2.MyClass[] { c1 };
                    Test.Inner.MyInterface_OpMyClassSeqResult opMyClassSeqResult = await i.opMyClassSeqAsync(cseq1);
                    test(opMyClassSeqResult.returnValue[0].s.Equals(s1));
                    test(opMyClassSeqResult.c2[0].s.Equals(s1));

                    var cmap1 = new Dictionary<string, Test.Inner.Inner2.MyClass>
                    {
                        ["a"] = c1
                    };
                    Test.Inner.MyInterface_OpMyClassMapResult opMyClassMapResult = await i.opMyClassMapAsync(cmap1);
                    test(opMyClassMapResult.returnValue["a"].s.Equals(s1));
                    test(opMyClassMapResult.c2["a"].s.Equals(s1));
                }).Wait();
        }

        {
            Test.Inner.Inner2.MyInterfacePrx i =
                Test.Inner.Inner2.MyInterfacePrxHelper.createProxy(communicator, "i3:" + helper.getTestEndpoint());

            var s1 = new Test.Inner.Inner2.MyStruct(0);
            Test.Inner.Inner2.MyStruct s3 = i.opMyStruct(s1, out Test.Inner.Inner2.MyStruct s2);
            test(s2.Equals(s1));
            test(s3.Equals(s1));

            var sseq1 = new Test.Inner.Inner2.MyStruct[] { s1 };
            Test.Inner.Inner2.MyStruct[] sseq3 = i.opMyStructSeq(sseq1, out Test.Inner.Inner2.MyStruct[] sseq2);
            test(sseq2[0].Equals(s1));
            test(sseq3[0].Equals(s1));

            var smap1 = new Dictionary<string, Test.Inner.Inner2.MyStruct>
            {
                ["a"] = s1
            };
            Dictionary<string, Test.Inner.Inner2.MyStruct> smap3 =
                i.opMyStructMap(smap1, out Dictionary<string, Test.Inner.Inner2.MyStruct> smap2);
            test(smap2["a"].Equals(s1));
            test(smap3["a"].Equals(s1));

            var c1 = new Test.Inner.Inner2.MyClass(s1);
            Test.Inner.Inner2.MyClass c3 = i.opMyClass(c1, out Test.Inner.Inner2.MyClass c2);
            test(c2.s.Equals(c1.s));
            test(c3.s.Equals(c1.s));

            var cseq1 = new Test.Inner.Inner2.MyClass[] { c1 };
            Test.Inner.Inner2.MyClass[] cseq3 = i.opMyClassSeq(cseq1, out Test.Inner.Inner2.MyClass[] cseq2);
            test(cseq2[0].s.Equals(s1));
            test(cseq3[0].s.Equals(s1));

            var cmap1 = new Dictionary<string, Test.Inner.Inner2.MyClass>
            {
                ["a"] = c1
            };
            Dictionary<string, Test.Inner.Inner2.MyClass> cmap3 =
                i.opMyClassMap(cmap1, out Dictionary<string, Test.Inner.Inner2.MyClass> cmap2);
            test(cmap2["a"].s.Equals(s1));
            test(cmap3["a"].s.Equals(s1));
        }

        {
            Test.Inner.Inner2.MyInterfacePrx i =
                Test.Inner.Inner2.MyInterfacePrxHelper.createProxy(communicator, "i3:" + helper.getTestEndpoint());

            Task.Run(async () =>
                {
                    var s1 = new Test.Inner.Inner2.MyStruct(0);
                    Test.Inner.Inner2.MyInterface_OpMyStructResult opMyStructResult = await i.opMyStructAsync(s1);
                    test(s1.Equals(opMyStructResult.returnValue));
                    test(s1.Equals(opMyStructResult.s2));

                    var sseq1 = new Test.Inner.Inner2.MyStruct[] { s1 };
                    Test.Inner.Inner2.MyInterface_OpMyStructSeqResult opMyStructSeqResult =
                        await i.opMyStructSeqAsync(sseq1);
                    test(opMyStructSeqResult.returnValue[0].Equals(s1));
                    test(opMyStructSeqResult.s2[0].Equals(s1));

                    var smap1 = new Dictionary<string, Test.Inner.Inner2.MyStruct>
                    {
                        ["a"] = s1
                    };
                    Test.Inner.Inner2.MyInterface_OpMyStructMapResult opMyStructMapResult =
                        await i.opMyStructMapAsync(smap1);
                    test(opMyStructMapResult.returnValue["a"].Equals(s1));
                    test(opMyStructMapResult.s2["a"].Equals(s1));

                    var c1 = new Test.Inner.Inner2.MyClass(s1);
                    Test.Inner.Inner2.MyInterface_OpMyClassResult opMyClassResult = await i.opMyClassAsync(c1);
                    test(c1.s.Equals(opMyClassResult.returnValue.s));
                    test(c1.s.Equals(opMyClassResult.c2.s));

                    var cseq1 = new Test.Inner.Inner2.MyClass[] { c1 };
                    Test.Inner.Inner2.MyInterface_OpMyClassSeqResult opMyClassSeqResult =
                        await i.opMyClassSeqAsync(cseq1);
                    test(opMyClassSeqResult.returnValue[0].s.Equals(s1));
                    test(opMyClassSeqResult.c2[0].s.Equals(s1));

                    var cmap1 = new Dictionary<string, Test.Inner.Inner2.MyClass>
                    {
                        ["a"] = c1
                    };
                    Test.Inner.Inner2.MyInterface_OpMyClassMapResult opMyClassMapResult =
                        await i.opMyClassMapAsync(cmap1);
                    test(opMyClassMapResult.returnValue["a"].s.Equals(s1));
                    test(opMyClassMapResult.c2["a"].s.Equals(s1));
                }).Wait();
        }

        {
            Inner.Test.Inner2.MyInterfacePrx i =
                Inner.Test.Inner2.MyInterfacePrxHelper.createProxy(communicator, "i4:" + helper.getTestEndpoint());

            var s1 = new Test.MyStruct(0);
            Test.MyStruct s3 = i.opMyStruct(s1, out Test.MyStruct s2);
            test(s2.Equals(s1));
            test(s3.Equals(s1));

            var sseq1 = new Test.MyStruct[] { s1 };
            Test.MyStruct[] sseq3 = i.opMyStructSeq(sseq1, out Test.MyStruct[] sseq2);
            test(sseq2[0].Equals(s1));
            test(sseq3[0].Equals(s1));

            var smap1 = new Dictionary<string, Test.MyStruct>
            {
                ["a"] = s1
            };
            Dictionary<string, Test.MyStruct> smap3 =
                i.opMyStructMap(smap1, out Dictionary<string, Test.MyStruct> smap2);
            test(smap2["a"].Equals(s1));
            test(smap3["a"].Equals(s1));

            var c1 = new Test.MyClass(s1);
            Test.MyClass c3 = i.opMyClass(c1, out Test.MyClass c2);
            test(c2.s.Equals(c1.s));
            test(c3.s.Equals(c1.s));

            var cseq1 = new Test.MyClass[] { c1 };
            Test.MyClass[] cseq3 = i.opMyClassSeq(cseq1, out Test.MyClass[] cseq2);
            test(cseq2[0].s.Equals(s1));
            test(cseq3[0].s.Equals(s1));

            var cmap1 = new Dictionary<string, Test.MyClass>
            {
                ["a"] = c1
            };
            Dictionary<string, Test.MyClass> cmap3 = i.opMyClassMap(cmap1, out Dictionary<string, Test.MyClass> cmap2);
            test(cmap2["a"].s.Equals(s1));
            test(cmap3["a"].s.Equals(s1));
        }

        {
            Inner.Test.Inner2.MyInterfacePrx i =
                Inner.Test.Inner2.MyInterfacePrxHelper.createProxy(communicator, "i4:" + helper.getTestEndpoint());

            Task.Run(async () =>
                {
                    var s1 = new Test.MyStruct(0);
                    Inner.Test.Inner2.MyInterface_OpMyStructResult opMyStructResult = await i.opMyStructAsync(s1);
                    test(s1.Equals(opMyStructResult.returnValue));
                    test(s1.Equals(opMyStructResult.s2));

                    var sseq1 = new Test.MyStruct[] { s1 };
                    Inner.Test.Inner2.MyInterface_OpMyStructSeqResult opMyStructSeqResult =
                        await i.opMyStructSeqAsync(sseq1);
                    test(opMyStructSeqResult.returnValue[0].Equals(s1));
                    test(opMyStructSeqResult.s2[0].Equals(s1));

                    var smap1 = new Dictionary<string, Test.MyStruct>
                    {
                        ["a"] = s1
                    };
                    Inner.Test.Inner2.MyInterface_OpMyStructMapResult opMyStructMapResult =
                        await i.opMyStructMapAsync(smap1);
                    test(opMyStructMapResult.returnValue["a"].Equals(s1));
                    test(opMyStructMapResult.s2["a"].Equals(s1));

                    var c1 = new Test.MyClass(s1);
                    Inner.Test.Inner2.MyInterface_OpMyClassResult opMyClassResult = await i.opMyClassAsync(c1);
                    test(c1.s.Equals(opMyClassResult.returnValue.s));
                    test(c1.s.Equals(opMyClassResult.c2.s));

                    var cseq1 = new Test.MyClass[] { c1 };
                    Inner.Test.Inner2.MyInterface_OpMyClassSeqResult opMyClassSeqResult =
                        await i.opMyClassSeqAsync(cseq1);
                    test(opMyClassSeqResult.returnValue[0].s.Equals(s1));
                    test(opMyClassSeqResult.c2[0].s.Equals(s1));

                    var cmap1 = new Dictionary<string, Test.MyClass>
                    {
                        ["a"] = c1
                    };
                    Inner.Test.Inner2.MyInterface_OpMyClassMapResult opMyClassMapResult =
                        await i.opMyClassMapAsync(cmap1);
                    test(opMyClassMapResult.returnValue["a"].s.Equals(s1));
                    test(opMyClassMapResult.c2["a"].s.Equals(s1));
                }).Wait();
        }

        {
            Test.MyInterfacePrx i = Test.MyInterfacePrxHelper.createProxy(communicator, "i1:" + helper.getTestEndpoint());
            i.shutdown();
        }
    }
}
