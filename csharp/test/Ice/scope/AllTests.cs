//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Threading.Tasks;
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
                    var obj = IObjectPrx.Parse($"i1:{helper.getTestEndpoint()}", communicator);
                    var i = Test.IPrx.CheckedCast(obj);

                    var s1 = new Test.S(0);
                    Test.S s2;
                    var s3 = i.opS(s1, out s2);
                    test(s2.Equals(s1));
                    test(s3.Equals(s1));

                    var sseq1 = new Test.S[] { s1 };
                    Test.S[] sseq2;
                    var sseq3 = i.opSSeq(sseq1, out sseq2);
                    test(sseq2[0].Equals(s1));
                    test(sseq3[0].Equals(s1));

                    Dictionary<String, Test.S> smap1 = new Dictionary<string, Test.S>();
                    smap1["a"] = s1;
                    Dictionary<String, Test.S> smap2;
                    Dictionary<String, Test.S> smap3 = i.opSMap(smap1, out smap2);
                    test(smap2["a"].Equals(s1));
                    test(smap3["a"].Equals(s1));

                    var c1 = new Test.C(s1);
                    Test.C c2;
                    var c3 = i.opC(c1, out c2);
                    test(c2.s.Equals(c1.s));
                    test(c3.s.Equals(c1.s));

                    var cseq1 = new Test.C[] { c1 };
                    Test.C[] cseq2;
                    var cseq3 = i.opCSeq(cseq1, out cseq2);
                    test(cseq2[0].s.Equals(s1));
                    test(cseq3[0].s.Equals(s1));

                    var cmap1 = new Dictionary<String, Test.C>();
                    cmap1["a"] = c1;
                    Dictionary<String, Test.C> cmap2;
                    var cmap3 = i.opCMap(cmap1, out cmap2);
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
                    var obj = IObjectPrx.Parse($"i1:{helper.getTestEndpoint()}", communicator);
                    var i = Test.IPrx.CheckedCast(obj);

                    Task.Run(async () =>
                        {
                            var s1 = new Test.S(0);
                            var opSResult = await i.opSAsync(s1);
                            test(s1.Equals(opSResult.returnValue));
                            test(s1.Equals(opSResult.s2));

                            var sseq1 = new Test.S[] { s1 };
                            var opSSeqResult = await i.opSSeqAsync(sseq1);
                            test(opSSeqResult.returnValue[0].Equals(s1));
                            test(opSSeqResult.s2[0].Equals(s1));

                            var smap1 = new Dictionary<String, Test.S>();
                            smap1["a"] = s1;
                            var opSMapResult = await i.opSMapAsync(smap1);
                            test(opSMapResult.returnValue["a"].Equals(s1));
                            test(opSMapResult.s2["a"].Equals(s1));

                            var c1 = new Test.C(s1);
                            var opCResult = await i.opCAsync(c1);
                            test(c1.s.Equals(opCResult.returnValue.s));
                            test(c1.s.Equals(opCResult.c2.s));

                            var cseq1 = new Test.C[] { c1 };
                            var opCSeqResult = await i.opCSeqAsync(cseq1);
                            test(opCSeqResult.returnValue[0].s.Equals(s1));
                            test(opCSeqResult.c2[0].s.Equals(s1));

                            var cmap1 = new Dictionary<String, Test.C>();
                            cmap1["a"] = c1;
                            var opCMapResult = await i.opCMapAsync(cmap1);
                            test(opCMapResult.returnValue["a"].s.Equals(s1));
                            test(opCMapResult.c2["a"].s.Equals(s1));

                            var e = await i.opE1Async(Test.E1.v1);
                            test(e == Test.E1.v1);

                            var s = await i.opS1Async(new Test.S1("S1"));
                            test(s.s == "S1");

                            var c = await i.opC1Async(new Test.C1("C1"));
                            test(c.s == "C1");
                        }).Wait();
                }

                {
                    var obj = IObjectPrx.Parse($"i2:{helper.getTestEndpoint()}", communicator);
                    Test.Inner.IPrx i = Test.Inner.IPrx.CheckedCast(obj);

                    Test.Inner.Inner2.S s1 = new Test.Inner.Inner2.S(0);
                    Test.Inner.Inner2.S s2;
                    Test.Inner.Inner2.S s3 = i.opS(s1, out s2);
                    test(s2.Equals(s1));
                    test(s3.Equals(s1));

                    Test.Inner.Inner2.S[] sseq1 = new Test.Inner.Inner2.S[] { s1 };
                    Test.Inner.Inner2.S[] sseq2;
                    Test.Inner.Inner2.S[] sseq3 = i.opSSeq(sseq1, out sseq2);
                    test(sseq2[0].Equals(s1));
                    test(sseq3[0].Equals(s1));

                    Dictionary<String, Test.Inner.Inner2.S> smap1 = new Dictionary<String, Test.Inner.Inner2.S>();
                    smap1["a"] = s1;
                    Dictionary<String, Test.Inner.Inner2.S> smap2;
                    Dictionary<String, Test.Inner.Inner2.S> smap3 = i.opSMap(smap1, out smap2);
                    test(smap2["a"].Equals(s1));
                    test(smap3["a"].Equals(s1));

                    Test.Inner.Inner2.C c1 = new Test.Inner.Inner2.C(s1);
                    Test.Inner.Inner2.C c2;
                    Test.Inner.Inner2.C c3 = i.opC(c1, out c2);
                    test(c2.s.Equals(c1.s));
                    test(c3.s.Equals(c1.s));

                    Test.Inner.Inner2.C[] cseq1 = new Test.Inner.Inner2.C[] { c1 };
                    Test.Inner.Inner2.C[] cseq2;
                    Test.Inner.Inner2.C[] cseq3 = i.opCSeq(cseq1, out cseq2);
                    test(cseq2[0].s.Equals(s1));
                    test(cseq3[0].s.Equals(s1));

                    Dictionary<String, Test.Inner.Inner2.C> cmap1 = new Dictionary<String, Test.Inner.Inner2.C>();
                    cmap1["a"] = c1;
                    Dictionary<String, Test.Inner.Inner2.C> cmap2;
                    Dictionary<String, Test.Inner.Inner2.C> cmap3 = i.opCMap(cmap1, out cmap2);
                    test(cmap2["a"].s.Equals(s1));
                    test(cmap3["a"].s.Equals(s1));
                }

                {
                    var obj = IObjectPrx.Parse($"i2:{helper.getTestEndpoint()}", communicator);
                    Test.Inner.IPrx i = Test.Inner.IPrx.CheckedCast(obj);

                    Task.Run(async () =>
                        {
                            Test.Inner.Inner2.S s1 = new Test.Inner.Inner2.S(0);
                            var opSResult = await i.opSAsync(s1);
                            test(s1.Equals(opSResult.returnValue));
                            test(s1.Equals(opSResult.s2));

                            Test.Inner.Inner2.S[] sseq1 = new Test.Inner.Inner2.S[] { s1 };
                            var opSSeqResult = await i.opSSeqAsync(sseq1);
                            test(opSSeqResult.returnValue[0].Equals(s1));
                            test(opSSeqResult.s2[0].Equals(s1));

                            Dictionary<String, Test.Inner.Inner2.S> smap1 = new Dictionary<String, Test.Inner.Inner2.S>();
                            smap1["a"] = s1;
                            var opSMapResult = await i.opSMapAsync(smap1);
                            test(opSMapResult.returnValue["a"].Equals(s1));
                            test(opSMapResult.s2["a"].Equals(s1));

                            Test.Inner.Inner2.C c1 = new Test.Inner.Inner2.C(s1);
                            var opCResult = await i.opCAsync(c1);
                            test(c1.s.Equals(opCResult.returnValue.s));
                            test(c1.s.Equals(opCResult.c2.s));

                            Test.Inner.Inner2.C[] cseq1 = new Test.Inner.Inner2.C[] { c1 };
                            var opCSeqResult = await i.opCSeqAsync(cseq1);
                            test(opCSeqResult.returnValue[0].s.Equals(s1));
                            test(opCSeqResult.c2[0].s.Equals(s1));

                            Dictionary<String, Test.Inner.Inner2.C> cmap1 = new Dictionary<String, Test.Inner.Inner2.C>();
                            cmap1["a"] = c1;
                            var opCMapResult = await i.opCMapAsync(cmap1);
                            test(opCMapResult.returnValue["a"].s.Equals(s1));
                            test(opCMapResult.c2["a"].s.Equals(s1));
                        }).Wait();
                }

                {
                    var obj = IObjectPrx.Parse($"i3:{helper.getTestEndpoint()}", communicator);
                    Test.Inner.Inner2.IPrx i = Test.Inner.Inner2.IPrx.CheckedCast(obj);

                    Test.Inner.Inner2.S s1 = new Test.Inner.Inner2.S(0);
                    Test.Inner.Inner2.S s2;
                    Test.Inner.Inner2.S s3 = i.opS(s1, out s2);
                    test(s2.Equals(s1));
                    test(s3.Equals(s1));

                    Test.Inner.Inner2.S[] sseq1 = new Test.Inner.Inner2.S[] { s1 };
                    Test.Inner.Inner2.S[] sseq2;
                    Test.Inner.Inner2.S[] sseq3 = i.opSSeq(sseq1, out sseq2);
                    test(sseq2[0].Equals(s1));
                    test(sseq3[0].Equals(s1));

                    Dictionary<String, Test.Inner.Inner2.S> smap1 = new Dictionary<String, Test.Inner.Inner2.S>();
                    smap1["a"] = s1;
                    Dictionary<String, Test.Inner.Inner2.S> smap2;
                    Dictionary<String, Test.Inner.Inner2.S> smap3 = i.opSMap(smap1, out smap2);
                    test(smap2["a"].Equals(s1));
                    test(smap3["a"].Equals(s1));

                    Test.Inner.Inner2.C c1 = new Test.Inner.Inner2.C(s1);
                    Test.Inner.Inner2.C c2;
                    Test.Inner.Inner2.C c3 = i.opC(c1, out c2);
                    test(c2.s.Equals(c1.s));
                    test(c3.s.Equals(c1.s));

                    Test.Inner.Inner2.C[] cseq1 = new Test.Inner.Inner2.C[] { c1 };
                    Test.Inner.Inner2.C[] cseq2;
                    Test.Inner.Inner2.C[] cseq3 = i.opCSeq(cseq1, out cseq2);
                    test(cseq2[0].s.Equals(s1));
                    test(cseq3[0].s.Equals(s1));

                    Dictionary<String, Test.Inner.Inner2.C> cmap1 = new Dictionary<String, Test.Inner.Inner2.C>();
                    cmap1["a"] = c1;
                    Dictionary<String, Test.Inner.Inner2.C> cmap2;
                    Dictionary<String, Test.Inner.Inner2.C> cmap3 = i.opCMap(cmap1, out cmap2);
                    test(cmap2["a"].s.Equals(s1));
                    test(cmap3["a"].s.Equals(s1));
                }

                {
                    var obj = IObjectPrx.Parse($"i3:{helper.getTestEndpoint()}", communicator);
                    Test.Inner.Inner2.IPrx? i = Test.Inner.Inner2.IPrx.CheckedCast(obj);
                    System.Diagnostics.Debug.Assert(i != null);

                    Task.Run(async () =>
                        {
                            Test.Inner.Inner2.S s1 = new Test.Inner.Inner2.S(0);
                            var opSResult = await i.opSAsync(s1);
                            test(s1.Equals(opSResult.returnValue));
                            test(s1.Equals(opSResult.s2));

                            Test.Inner.Inner2.S[] sseq1 = new Test.Inner.Inner2.S[] { s1 };
                            var opSSeqResult = await i.opSSeqAsync(sseq1);
                            test(opSSeqResult.returnValue[0].Equals(s1));
                            test(opSSeqResult.s2[0].Equals(s1));

                            Dictionary<String, Test.Inner.Inner2.S> smap1 = new Dictionary<String, Test.Inner.Inner2.S>();
                            smap1["a"] = s1;
                            var opSMapResult = await i.opSMapAsync(smap1);
                            test(opSMapResult.returnValue["a"].Equals(s1));
                            test(opSMapResult.s2["a"].Equals(s1));

                            Test.Inner.Inner2.C c1 = new Test.Inner.Inner2.C(s1);
                            var opCResult = await i.opCAsync(c1);
                            test(c1.s.Equals(opCResult.returnValue.s));
                            test(c1.s.Equals(opCResult.c2.s));

                            Test.Inner.Inner2.C[] cseq1 = new Test.Inner.Inner2.C[] { c1 };
                            var opCSeqResult = await i.opCSeqAsync(cseq1);
                            test(opCSeqResult.returnValue[0].s.Equals(s1));
                            test(opCSeqResult.c2[0].s.Equals(s1));

                            Dictionary<String, Test.Inner.Inner2.C> cmap1 = new Dictionary<String, Test.Inner.Inner2.C>();
                            cmap1["a"] = c1;
                            var opCMapResult = await i.opCMapAsync(cmap1);
                            test(opCMapResult.returnValue["a"].s.Equals(s1));
                            test(opCMapResult.c2["a"].s.Equals(s1));
                        }).Wait();
                }

                {
                    var obj = IObjectPrx.Parse($"i4:{helper.getTestEndpoint()}", communicator);
                    Inner.Test.Inner2.IPrx i = Inner.Test.Inner2.IPrx.CheckedCast(obj);

                    Test.S s1 = new Test.S(0);
                    Test.S s2;
                    Test.S s3 = i.opS(s1, out s2);
                    test(s2.Equals(s1));
                    test(s3.Equals(s1));

                    Test.S[] sseq1 = new Test.S[] { s1 };
                    Test.S[] sseq2;
                    Test.S[] sseq3 = i.opSSeq(sseq1, out sseq2);
                    test(sseq2[0].Equals(s1));
                    test(sseq3[0].Equals(s1));

                    Dictionary<String, Test.S> smap1 = new Dictionary<String, Test.S>();
                    smap1["a"] = s1;
                    Dictionary<String, Test.S> smap2;
                    Dictionary<String, Test.S> smap3 = i.opSMap(smap1, out smap2);
                    test(smap2["a"].Equals(s1));
                    test(smap3["a"].Equals(s1));

                    Test.C c1 = new Test.C(s1);
                    Test.C c2;
                    Test.C c3 = i.opC(c1, out c2);
                    test(c2.s.Equals(c1.s));
                    test(c3.s.Equals(c1.s));

                    Test.C[] cseq1 = new Test.C[] { c1 };
                    Test.C[] cseq2;
                    Test.C[] cseq3 = i.opCSeq(cseq1, out cseq2);
                    test(cseq2[0].s.Equals(s1));
                    test(cseq3[0].s.Equals(s1));

                    Dictionary<String, Test.C> cmap1 = new Dictionary<String, Test.C>();
                    cmap1["a"] = c1;
                    Dictionary<String, Test.C> cmap2;
                    Dictionary<String, Test.C> cmap3 = i.opCMap(cmap1, out cmap2);
                    test(cmap2["a"].s.Equals(s1));
                    test(cmap3["a"].s.Equals(s1));
                }

                {
                    var obj = IObjectPrx.Parse($"i4:{helper.getTestEndpoint()}", communicator);
                    Inner.Test.Inner2.IPrx i = Inner.Test.Inner2.IPrx.CheckedCast(obj);

                    Task.Run(async () =>
                        {
                            Test.S s1 = new Test.S(0);
                            var opSResult = await i.opSAsync(s1);
                            test(s1.Equals(opSResult.returnValue));
                            test(s1.Equals(opSResult.s2));

                            Test.S[] sseq1 = new Test.S[] { s1 };
                            var opSSeqResult = await i.opSSeqAsync(sseq1);
                            test(opSSeqResult.returnValue[0].Equals(s1));
                            test(opSSeqResult.s2[0].Equals(s1));

                            Dictionary<String, Test.S> smap1 = new Dictionary<String, Test.S>();
                            smap1["a"] = s1;
                            var opSMapResult = await i.opSMapAsync(smap1);
                            test(opSMapResult.returnValue["a"].Equals(s1));
                            test(opSMapResult.s2["a"].Equals(s1));

                            Test.C c1 = new Test.C(s1);
                            var opCResult = await i.opCAsync(c1);
                            test(c1.s.Equals(opCResult.returnValue.s));
                            test(c1.s.Equals(opCResult.c2.s));

                            Test.C[] cseq1 = new Test.C[] { c1 };
                            var opCSeqResult = await i.opCSeqAsync(cseq1);
                            test(opCSeqResult.returnValue[0].s.Equals(s1));
                            test(opCSeqResult.c2[0].s.Equals(s1));

                            Dictionary<String, Test.C> cmap1 = new Dictionary<String, Test.C>();
                            cmap1["a"] = c1;
                            var opCMapResult = await i.opCMapAsync(cmap1);
                            test(opCMapResult.returnValue["a"].s.Equals(s1));
                            test(opCMapResult.c2["a"].s.Equals(s1));
                        }).Wait();
                }

                {
                    Test.IPrx.Parse($"i1:{helper.getTestEndpoint()}", communicator).shutdown();
                }
            }
        }
    }
}
