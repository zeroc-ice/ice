// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice
{
    namespace scope
    {
        public class AllTests : global::Test.AllTests
        {
            public static void allTests(TestHelper helper)
            {
                var communicator = helper.communicator();
                {
                    var i = Test.IPrxHelper.createProxy(communicator, "i1:" + helper.getTestEndpoint());

                    var s1 = new Test.MyStruct(0);
                    Test.MyStruct s2;
                    var s3 = i.opMyStruct(s1, out s2);
                    test(s2.Equals(s1));
                    test(s3.Equals(s1));

                    var sseq1 = new Test.MyStruct[] { s1 };
                    Test.MyStruct[] sseq2;
                    var sseq3 = i.opMyStructSeq(sseq1, out sseq2);
                    test(sseq2[0].Equals(s1));
                    test(sseq3[0].Equals(s1));

                    Dictionary<String, Test.MyStruct> smap1 = new Dictionary<string, Test.MyStruct>();
                    smap1["a"] = s1;
                    Dictionary<String, Test.MyStruct> smap2;
                    Dictionary<String, Test.MyStruct> smap3 = i.opMyStructMap(smap1, out smap2);
                    test(smap2["a"].Equals(s1));
                    test(smap3["a"].Equals(s1));

                    var c1 = new Test.MyClass(s1);
                    Test.MyClass c2;
                    var c3 = i.opMyClass(c1, out c2);
                    test(c2.s.Equals(c1.s));
                    test(c3.s.Equals(c1.s));

                    var cseq1 = new Test.MyClass[] { c1 };
                    Test.MyClass[] cseq2;
                    var cseq3 = i.opMyClassSeq(cseq1, out cseq2);
                    test(cseq2[0].s.Equals(s1));
                    test(cseq3[0].s.Equals(s1));

                    var cmap1 = new Dictionary<String, Test.MyClass>();
                    cmap1["a"] = c1;
                    Dictionary<String, Test.MyClass> cmap2;
                    var cmap3 = i.opMyClassMap(cmap1, out cmap2);
                    test(cmap2["a"].s.Equals(s1));
                    test(cmap3["a"].s.Equals(s1));

                    var e = i.opE1(Test.E1.v1);
                    test(e == Test.E1.v1);

                    var s = i.opS1(new Test.S1("S1"));
                    test(s.s == "S1");

                    var c = i.opC1(new Test.C1("C1"));
                    test(c.s == "C1");
                }

                {
                    var i = Test.IPrxHelper.createProxy(communicator, "i1:" + helper.getTestEndpoint());

                    Task.Run(async () =>
                        {
                            var s1 = new Test.MyStruct(0);
                            var opMyStructResult = await i.opMyStructAsync(s1);
                            test(s1.Equals(opMyStructResult.returnValue));
                            test(s1.Equals(opMyStructResult.s2));

                            var sseq1 = new Test.MyStruct[] { s1 };
                            var opMyStructSeqResult = await i.opMyStructSeqAsync(sseq1);
                            test(opMyStructSeqResult.returnValue[0].Equals(s1));
                            test(opMyStructSeqResult.s2[0].Equals(s1));

                            var smap1 = new Dictionary<String, Test.MyStruct>();
                            smap1["a"] = s1;
                            var opMyStructMapResult = await i.opMyStructMapAsync(smap1);
                            test(opMyStructMapResult.returnValue["a"].Equals(s1));
                            test(opMyStructMapResult.s2["a"].Equals(s1));

                            var c1 = new Test.MyClass(s1);
                            var opMyClassResult = await i.opMyClassAsync(c1);
                            test(c1.s.Equals(opMyClassResult.returnValue.s));
                            test(c1.s.Equals(opMyClassResult.c2.s));

                            var cseq1 = new Test.MyClass[] { c1 };
                            var opMyClassSeqResult = await i.opMyClassSeqAsync(cseq1);
                            test(opMyClassSeqResult.returnValue[0].s.Equals(s1));
                            test(opMyClassSeqResult.c2[0].s.Equals(s1));

                            var cmap1 = new Dictionary<String, Test.MyClass>();
                            cmap1["a"] = c1;
                            var opMyClassMapResult = await i.opMyClassMapAsync(cmap1);
                            test(opMyClassMapResult.returnValue["a"].s.Equals(s1));
                            test(opMyClassMapResult.c2["a"].s.Equals(s1));

                            var e = await i.opE1Async(Test.E1.v1);
                            test(e == Test.E1.v1);

                            var s = await i.opS1Async(new Test.S1("S1"));
                            test(s.s == "S1");

                            var c = await i.opC1Async(new Test.C1("C1"));
                            test(c.s == "C1");
                        }).Wait();
                }

                {
                    var i = Test.Inner.IPrxHelper.createProxy(communicator, "i2:" + helper.getTestEndpoint());

                    Test.Inner.Inner2.MyStruct s1 = new Test.Inner.Inner2.MyStruct(0);
                    Test.Inner.Inner2.MyStruct s2;
                    Test.Inner.Inner2.MyStruct s3 = i.opMyStruct(s1, out s2);
                    test(s2.Equals(s1));
                    test(s3.Equals(s1));

                    Test.Inner.Inner2.MyStruct[] sseq1 = new Test.Inner.Inner2.MyStruct[] { s1 };
                    Test.Inner.Inner2.MyStruct[] sseq2;
                    Test.Inner.Inner2.MyStruct[] sseq3 = i.opMyStructSeq(sseq1, out sseq2);
                    test(sseq2[0].Equals(s1));
                    test(sseq3[0].Equals(s1));

                    Dictionary<String, Test.Inner.Inner2.MyStruct> smap1 = new Dictionary<String, Test.Inner.Inner2.MyStruct>();
                    smap1["a"] = s1;
                    Dictionary<String, Test.Inner.Inner2.MyStruct> smap2;
                    Dictionary<String, Test.Inner.Inner2.MyStruct> smap3 = i.opMyStructMap(smap1, out smap2);
                    test(smap2["a"].Equals(s1));
                    test(smap3["a"].Equals(s1));

                    Test.Inner.Inner2.MyClass c1 = new Test.Inner.Inner2.MyClass(s1);
                    Test.Inner.Inner2.MyClass c2;
                    Test.Inner.Inner2.MyClass c3 = i.opMyClass(c1, out c2);
                    test(c2.s.Equals(c1.s));
                    test(c3.s.Equals(c1.s));

                    Test.Inner.Inner2.MyClass[] cseq1 = new Test.Inner.Inner2.MyClass[] { c1 };
                    Test.Inner.Inner2.MyClass[] cseq2;
                    Test.Inner.Inner2.MyClass[] cseq3 = i.opMyClassSeq(cseq1, out cseq2);
                    test(cseq2[0].s.Equals(s1));
                    test(cseq3[0].s.Equals(s1));

                    Dictionary<String, Test.Inner.Inner2.MyClass> cmap1 = new Dictionary<String, Test.Inner.Inner2.MyClass>();
                    cmap1["a"] = c1;
                    Dictionary<String, Test.Inner.Inner2.MyClass> cmap2;
                    Dictionary<String, Test.Inner.Inner2.MyClass> cmap3 = i.opMyClassMap(cmap1, out cmap2);
                    test(cmap2["a"].s.Equals(s1));
                    test(cmap3["a"].s.Equals(s1));
                }

                {
                    var i = Test.Inner.IPrxHelper.createProxy(communicator, "i2:" + helper.getTestEndpoint());

                    Task.Run(async () =>
                        {
                            Test.Inner.Inner2.MyStruct s1 = new Test.Inner.Inner2.MyStruct(0);
                            Test.Inner.I_OpMyStructResult opMyStructResult = await i.opMyStructAsync(s1);
                            test(s1.Equals(opMyStructResult.returnValue));
                            test(s1.Equals(opMyStructResult.s2));

                            Test.Inner.Inner2.MyStruct[] sseq1 = new Test.Inner.Inner2.MyStruct[] { s1 };
                            Test.Inner.I_OpMyStructSeqResult opMyStructSeqResult = await i.opMyStructSeqAsync(sseq1);
                            test(opMyStructSeqResult.returnValue[0].Equals(s1));
                            test(opMyStructSeqResult.s2[0].Equals(s1));

                            Dictionary<String, Test.Inner.Inner2.MyStruct> smap1 = new Dictionary<String, Test.Inner.Inner2.MyStruct>();
                            smap1["a"] = s1;
                            Test.Inner.I_OpMyStructMapResult opMyStructMapResult = await i.opMyStructMapAsync(smap1);
                            test(opMyStructMapResult.returnValue["a"].Equals(s1));
                            test(opMyStructMapResult.s2["a"].Equals(s1));

                            Test.Inner.Inner2.MyClass c1 = new Test.Inner.Inner2.MyClass(s1);
                            Test.Inner.I_OpMyClassResult opMyClassResult = await i.opMyClassAsync(c1);
                            test(c1.s.Equals(opMyClassResult.returnValue.s));
                            test(c1.s.Equals(opMyClassResult.c2.s));

                            Test.Inner.Inner2.MyClass[] cseq1 = new Test.Inner.Inner2.MyClass[] { c1 };
                            Test.Inner.I_OpMyClassSeqResult opMyClassSeqResult = await i.opMyClassSeqAsync(cseq1);
                            test(opMyClassSeqResult.returnValue[0].s.Equals(s1));
                            test(opMyClassSeqResult.c2[0].s.Equals(s1));

                            Dictionary<String, Test.Inner.Inner2.MyClass> cmap1 = new Dictionary<String, Test.Inner.Inner2.MyClass>();
                            cmap1["a"] = c1;
                            Test.Inner.I_OpMyClassMapResult opMyClassMapResult = await i.opMyClassMapAsync(cmap1);
                            test(opMyClassMapResult.returnValue["a"].s.Equals(s1));
                            test(opMyClassMapResult.c2["a"].s.Equals(s1));
                        }).Wait();
                }

                {
                    var i = Test.Inner.Inner2.IPrxHelper.createProxy(communicator, "i3:" + helper.getTestEndpoint());

                    Test.Inner.Inner2.MyStruct s1 = new Test.Inner.Inner2.MyStruct(0);
                    Test.Inner.Inner2.MyStruct s2;
                    Test.Inner.Inner2.MyStruct s3 = i.opMyStruct(s1, out s2);
                    test(s2.Equals(s1));
                    test(s3.Equals(s1));

                    Test.Inner.Inner2.MyStruct[] sseq1 = new Test.Inner.Inner2.MyStruct[] { s1 };
                    Test.Inner.Inner2.MyStruct[] sseq2;
                    Test.Inner.Inner2.MyStruct[] sseq3 = i.opMyStructSeq(sseq1, out sseq2);
                    test(sseq2[0].Equals(s1));
                    test(sseq3[0].Equals(s1));

                    Dictionary<String, Test.Inner.Inner2.MyStruct> smap1 = new Dictionary<String, Test.Inner.Inner2.MyStruct>();
                    smap1["a"] = s1;
                    Dictionary<String, Test.Inner.Inner2.MyStruct> smap2;
                    Dictionary<String, Test.Inner.Inner2.MyStruct> smap3 = i.opMyStructMap(smap1, out smap2);
                    test(smap2["a"].Equals(s1));
                    test(smap3["a"].Equals(s1));

                    Test.Inner.Inner2.MyClass c1 = new Test.Inner.Inner2.MyClass(s1);
                    Test.Inner.Inner2.MyClass c2;
                    Test.Inner.Inner2.MyClass c3 = i.opMyClass(c1, out c2);
                    test(c2.s.Equals(c1.s));
                    test(c3.s.Equals(c1.s));

                    Test.Inner.Inner2.MyClass[] cseq1 = new Test.Inner.Inner2.MyClass[] { c1 };
                    Test.Inner.Inner2.MyClass[] cseq2;
                    Test.Inner.Inner2.MyClass[] cseq3 = i.opMyClassSeq(cseq1, out cseq2);
                    test(cseq2[0].s.Equals(s1));
                    test(cseq3[0].s.Equals(s1));

                    Dictionary<String, Test.Inner.Inner2.MyClass> cmap1 = new Dictionary<String, Test.Inner.Inner2.MyClass>();
                    cmap1["a"] = c1;
                    Dictionary<String, Test.Inner.Inner2.MyClass> cmap2;
                    Dictionary<String, Test.Inner.Inner2.MyClass> cmap3 = i.opMyClassMap(cmap1, out cmap2);
                    test(cmap2["a"].s.Equals(s1));
                    test(cmap3["a"].s.Equals(s1));
                }

                {
                    var i = Test.Inner.Inner2.IPrxHelper.createProxy(communicator, "i3:" + helper.getTestEndpoint());

                    Task.Run(async () =>
                        {
                            Test.Inner.Inner2.MyStruct s1 = new Test.Inner.Inner2.MyStruct(0);
                            Test.Inner.Inner2.I_OpMyStructResult opMyStructResult = await i.opMyStructAsync(s1);
                            test(s1.Equals(opMyStructResult.returnValue));
                            test(s1.Equals(opMyStructResult.s2));

                            Test.Inner.Inner2.MyStruct[] sseq1 = new Test.Inner.Inner2.MyStruct[] { s1 };
                            Test.Inner.Inner2.I_OpMyStructSeqResult opMyStructSeqResult = await i.opMyStructSeqAsync(sseq1);
                            test(opMyStructSeqResult.returnValue[0].Equals(s1));
                            test(opMyStructSeqResult.s2[0].Equals(s1));

                            Dictionary<String, Test.Inner.Inner2.MyStruct> smap1 = new Dictionary<String, Test.Inner.Inner2.MyStruct>();
                            smap1["a"] = s1;
                            Test.Inner.Inner2.I_OpMyStructMapResult opMyStructMapResult = await i.opMyStructMapAsync(smap1);
                            test(opMyStructMapResult.returnValue["a"].Equals(s1));
                            test(opMyStructMapResult.s2["a"].Equals(s1));

                            Test.Inner.Inner2.MyClass c1 = new Test.Inner.Inner2.MyClass(s1);
                            Test.Inner.Inner2.I_OpMyClassResult opMyClassResult = await i.opMyClassAsync(c1);
                            test(c1.s.Equals(opMyClassResult.returnValue.s));
                            test(c1.s.Equals(opMyClassResult.c2.s));

                            Test.Inner.Inner2.MyClass[] cseq1 = new Test.Inner.Inner2.MyClass[] { c1 };
                            Test.Inner.Inner2.I_OpMyClassSeqResult opMyClassSeqResult = await i.opMyClassSeqAsync(cseq1);
                            test(opMyClassSeqResult.returnValue[0].s.Equals(s1));
                            test(opMyClassSeqResult.c2[0].s.Equals(s1));

                            Dictionary<String, Test.Inner.Inner2.MyClass> cmap1 = new Dictionary<String, Test.Inner.Inner2.MyClass>();
                            cmap1["a"] = c1;
                            Test.Inner.Inner2.I_OpMyClassMapResult opMyClassMapResult = await i.opMyClassMapAsync(cmap1);
                            test(opMyClassMapResult.returnValue["a"].s.Equals(s1));
                            test(opMyClassMapResult.c2["a"].s.Equals(s1));
                        }).Wait();
                }

                {
                    var i = Inner.Test.Inner2.IPrxHelper.createProxy(communicator, "i4:" + helper.getTestEndpoint());

                    Test.MyStruct s1 = new Test.MyStruct(0);
                    Test.MyStruct s2;
                    Test.MyStruct s3 = i.opMyStruct(s1, out s2);
                    test(s2.Equals(s1));
                    test(s3.Equals(s1));

                    Test.MyStruct[] sseq1 = new Test.MyStruct[] { s1 };
                    Test.MyStruct[] sseq2;
                    Test.MyStruct[] sseq3 = i.opMyStructSeq(sseq1, out sseq2);
                    test(sseq2[0].Equals(s1));
                    test(sseq3[0].Equals(s1));

                    Dictionary<String, Test.MyStruct> smap1 = new Dictionary<String, Test.MyStruct>();
                    smap1["a"] = s1;
                    Dictionary<String, Test.MyStruct> smap2;
                    Dictionary<String, Test.MyStruct> smap3 = i.opMyStructMap(smap1, out smap2);
                    test(smap2["a"].Equals(s1));
                    test(smap3["a"].Equals(s1));

                    Test.MyClass c1 = new Test.MyClass(s1);
                    Test.MyClass c2;
                    Test.MyClass c3 = i.opMyClass(c1, out c2);
                    test(c2.s.Equals(c1.s));
                    test(c3.s.Equals(c1.s));

                    Test.MyClass[] cseq1 = new Test.MyClass[] { c1 };
                    Test.MyClass[] cseq2;
                    Test.MyClass[] cseq3 = i.opMyClassSeq(cseq1, out cseq2);
                    test(cseq2[0].s.Equals(s1));
                    test(cseq3[0].s.Equals(s1));

                    Dictionary<String, Test.MyClass> cmap1 = new Dictionary<String, Test.MyClass>();
                    cmap1["a"] = c1;
                    Dictionary<String, Test.MyClass> cmap2;
                    Dictionary<String, Test.MyClass> cmap3 = i.opMyClassMap(cmap1, out cmap2);
                    test(cmap2["a"].s.Equals(s1));
                    test(cmap3["a"].s.Equals(s1));
                }

                {
                    var i = Inner.Test.Inner2.IPrxHelper.createProxy(communicator, "i4:" + helper.getTestEndpoint());

                    Task.Run(async () =>
                        {
                            Test.MyStruct s1 = new Test.MyStruct(0);
                            Inner.Test.Inner2.I_OpMyStructResult opMyStructResult = await i.opMyStructAsync(s1);
                            test(s1.Equals(opMyStructResult.returnValue));
                            test(s1.Equals(opMyStructResult.s2));

                            Test.MyStruct[] sseq1 = new Test.MyStruct[] { s1 };
                            Inner.Test.Inner2.I_OpMyStructSeqResult opMyStructSeqResult = await i.opMyStructSeqAsync(sseq1);
                            test(opMyStructSeqResult.returnValue[0].Equals(s1));
                            test(opMyStructSeqResult.s2[0].Equals(s1));

                            Dictionary<String, Test.MyStruct> smap1 = new Dictionary<String, Test.MyStruct>();
                            smap1["a"] = s1;
                            Inner.Test.Inner2.I_OpMyStructMapResult opMyStructMapResult = await i.opMyStructMapAsync(smap1);
                            test(opMyStructMapResult.returnValue["a"].Equals(s1));
                            test(opMyStructMapResult.s2["a"].Equals(s1));

                            Test.MyClass c1 = new Test.MyClass(s1);
                            Inner.Test.Inner2.I_OpMyClassResult opMyClassResult = await i.opMyClassAsync(c1);
                            test(c1.s.Equals(opMyClassResult.returnValue.s));
                            test(c1.s.Equals(opMyClassResult.c2.s));

                            Test.MyClass[] cseq1 = new Test.MyClass[] { c1 };
                            Inner.Test.Inner2.I_OpMyClassSeqResult opMyClassSeqResult = await i.opMyClassSeqAsync(cseq1);
                            test(opMyClassSeqResult.returnValue[0].s.Equals(s1));
                            test(opMyClassSeqResult.c2[0].s.Equals(s1));

                            Dictionary<String, Test.MyClass> cmap1 = new Dictionary<String, Test.MyClass>();
                            cmap1["a"] = c1;
                            Inner.Test.Inner2.I_OpMyClassMapResult opMyClassMapResult = await i.opMyClassMapAsync(cmap1);
                            test(opMyClassMapResult.returnValue["a"].s.Equals(s1));
                            test(opMyClassMapResult.c2["a"].s.Equals(s1));
                        }).Wait();
                }

                {
                    var i = Test.IPrxHelper.createProxy(communicator, "i1:" + helper.getTestEndpoint());
                    i.shutdown();
                }
            }
        }
    }
}
