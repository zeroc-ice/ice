//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Test;

namespace Ice.scope
{
    public class AllTests : global::Test.AllTests
    {
        public static void allTests(TestHelper helper)
        {
            var communicator = helper.communicator();
            {
                var obj = IObjectPrx.Parse($"i1:{helper.getTestEndpoint()}", communicator);
                var i = Test.IIPrx.CheckedCast(obj);

                var s1 = new Test.S(0);
                var (s3, s2) = i.opS(s1);
                test(s2.Equals(s1));
                test(s3.Equals(s1));

                var sseq1 = new Test.S[] { s1 };
                var (sseq3, sseq2) = i.opSSeq(sseq1);
                test(sseq2[0].Equals(s1));
                test(sseq3[0].Equals(s1));

                Dictionary<string, Test.S> smap1 = new Dictionary<string, Test.S>();
                smap1["a"] = s1;
                var (smap3, smap2) = i.opSMap(smap1);
                test(smap2["a"].Equals(s1));
                test(smap3["a"].Equals(s1));

                var c1 = new Test.C(s1);
                var (c3, c2) = i.opC(c1);
                test(c2.s.Equals(c1.s));
                test(c3.s.Equals(c1.s));

                var cseq1 = new Test.C[] { c1 };
                var (cseq3, cseq2) = i.opCSeq(cseq1);
                test(cseq2[0].s.Equals(s1));
                test(cseq3[0].s.Equals(s1));

                var cmap1 = new Dictionary<string, Test.C>();
                cmap1["a"] = c1;
                var (cmap3, cmap2) = i.opCMap(cmap1);
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
                var i = Test.IIPrx.CheckedCast(obj);

                Task.Run(async () =>
                    {
                        var s1 = new Test.S(0);
                        var opSResult = await i.opSAsync(s1);
                        test(s1.Equals(opSResult.ReturnValue));
                        test(s1.Equals(opSResult.s2));

                        var sseq1 = new Test.S[] { s1 };
                        var opSSeqResult = await i.opSSeqAsync(sseq1);
                        test(opSSeqResult.ReturnValue[0].Equals(s1));
                        test(opSSeqResult.s2[0].Equals(s1));

                        var smap1 = new Dictionary<string, Test.S>();
                        smap1["a"] = s1;
                        var opSMapResult = await i.opSMapAsync(smap1);
                        test(opSMapResult.ReturnValue["a"].Equals(s1));
                        test(opSMapResult.s2["a"].Equals(s1));

                        var c1 = new Test.C(s1);
                        var opCResult = await i.opCAsync(c1);
                        test(c1.s.Equals(opCResult.ReturnValue.s));
                        test(c1.s.Equals(opCResult.c2.s));

                        var cseq1 = new Test.C[] { c1 };
                        var opCSeqResult = await i.opCSeqAsync(cseq1);
                        test(opCSeqResult.ReturnValue[0].s.Equals(s1));
                        test(opCSeqResult.c2[0].s.Equals(s1));

                        var cmap1 = new Dictionary<String, Test.C>();
                        cmap1["a"] = c1;
                        var opCMapResult = await i.opCMapAsync(cmap1);
                        test(opCMapResult.ReturnValue["a"].s.Equals(s1));
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
                Test.Inner.IIPrx i = Test.Inner.IIPrx.CheckedCast(obj);

                Test.Inner.Inner2.S s1 = new Test.Inner.Inner2.S(0);
                var (s3, s2) = i.opS(s1);
                test(s2.Equals(s1));
                test(s3.Equals(s1));

                Test.Inner.Inner2.S[] sseq1 = new Test.Inner.Inner2.S[] { s1 };
                var (sseq3, sseq2) = i.opSSeq(sseq1);
                test(sseq2[0].Equals(s1));
                test(sseq3[0].Equals(s1));

                var smap1 = new Dictionary<string, Test.Inner.Inner2.S>();
                smap1["a"] = s1;
                var (smap3, smap2) = i.opSMap(smap1);
                test(smap2["a"].Equals(s1));
                test(smap3["a"].Equals(s1));

                var c1 = new Test.Inner.Inner2.C(s1);
                var (c3, c2) = i.opC(c1);
                test(c2.s.Equals(c1.s));
                test(c3.s.Equals(c1.s));

                var cseq1 = new Test.Inner.Inner2.C[] { c1 };
                var (cseq3, cseq2) = i.opCSeq(cseq1);
                test(cseq2[0].s.Equals(s1));
                test(cseq3[0].s.Equals(s1));

                var cmap1 = new Dictionary<string, Test.Inner.Inner2.C>();
                cmap1["a"] = c1;
                var (cmap3, cmap2) = i.opCMap(cmap1);
                test(cmap2["a"].s.Equals(s1));
                test(cmap3["a"].s.Equals(s1));
            }

            {
                var obj = IObjectPrx.Parse($"i2:{helper.getTestEndpoint()}", communicator);
                Test.Inner.IIPrx i = Test.Inner.IIPrx.CheckedCast(obj);

                Task.Run(async () =>
                    {
                        Test.Inner.Inner2.S s1 = new Test.Inner.Inner2.S(0);
                        var opSResult = await i.opSAsync(s1);
                        test(s1.Equals(opSResult.ReturnValue));
                        test(s1.Equals(opSResult.s2));

                        Test.Inner.Inner2.S[] sseq1 = new Test.Inner.Inner2.S[] { s1 };
                        var opSSeqResult = await i.opSSeqAsync(sseq1);
                        test(opSSeqResult.ReturnValue[0].Equals(s1));
                        test(opSSeqResult.s2[0].Equals(s1));

                        Dictionary<String, Test.Inner.Inner2.S> smap1 = new Dictionary<String, Test.Inner.Inner2.S>();
                        smap1["a"] = s1;
                        var opSMapResult = await i.opSMapAsync(smap1);
                        test(opSMapResult.ReturnValue["a"].Equals(s1));
                        test(opSMapResult.s2["a"].Equals(s1));

                        Test.Inner.Inner2.C c1 = new Test.Inner.Inner2.C(s1);
                        var opCResult = await i.opCAsync(c1);
                        test(c1.s.Equals(opCResult.ReturnValue.s));
                        test(c1.s.Equals(opCResult.c2.s));

                        Test.Inner.Inner2.C[] cseq1 = new Test.Inner.Inner2.C[] { c1 };
                        var opCSeqResult = await i.opCSeqAsync(cseq1);
                        test(opCSeqResult.ReturnValue[0].s.Equals(s1));
                        test(opCSeqResult.c2[0].s.Equals(s1));

                        Dictionary<String, Test.Inner.Inner2.C> cmap1 = new Dictionary<String, Test.Inner.Inner2.C>();
                        cmap1["a"] = c1;
                        var opCMapResult = await i.opCMapAsync(cmap1);
                        test(opCMapResult.ReturnValue["a"].s.Equals(s1));
                        test(opCMapResult.c2["a"].s.Equals(s1));
                    }).Wait();
            }

            {
                var obj = IObjectPrx.Parse($"i3:{helper.getTestEndpoint()}", communicator);
                Test.Inner.Inner2.IIPrx i = Test.Inner.Inner2.IIPrx.CheckedCast(obj);

                var s1 = new Test.Inner.Inner2.S(0);
                var (s3, s2) = i.opS(s1);
                test(s2.Equals(s1));
                test(s3.Equals(s1));

                var sseq1 = new Test.Inner.Inner2.S[] { s1 };
                var (sseq3, sseq2) = i.opSSeq(sseq1);
                test(sseq2[0].Equals(s1));
                test(sseq3[0].Equals(s1));

                var smap1 = new Dictionary<string, Test.Inner.Inner2.S>();
                smap1["a"] = s1;
                var (smap3, smap2) = i.opSMap(smap1);
                test(smap2["a"].Equals(s1));
                test(smap3["a"].Equals(s1));

                var c1 = new Test.Inner.Inner2.C(s1);
                var (c3, c2) = i.opC(c1);
                test(c2.s.Equals(c1.s));
                test(c3.s.Equals(c1.s));

                var cseq1 = new Test.Inner.Inner2.C[] { c1 };
                var (cseq3, cseq2) = i.opCSeq(cseq1);
                test(cseq2[0].s.Equals(s1));
                test(cseq3[0].s.Equals(s1));

                var cmap1 = new Dictionary<string, Test.Inner.Inner2.C>();
                cmap1["a"] = c1;
                var (cmap3, cmap2) = i.opCMap(cmap1);
                test(cmap2["a"].s.Equals(s1));
                test(cmap3["a"].s.Equals(s1));
            }

            {
                var obj = IObjectPrx.Parse($"i3:{helper.getTestEndpoint()}", communicator);
                Test.Inner.Inner2.IIPrx? i = Test.Inner.Inner2.IIPrx.CheckedCast(obj);
                System.Diagnostics.Debug.Assert(i != null);

                Task.Run(async () =>
                    {
                        Test.Inner.Inner2.S s1 = new Test.Inner.Inner2.S(0);
                        var opSResult = await i.opSAsync(s1);
                        test(s1.Equals(opSResult.ReturnValue));
                        test(s1.Equals(opSResult.s2));

                        Test.Inner.Inner2.S[] sseq1 = new Test.Inner.Inner2.S[] { s1 };
                        var opSSeqResult = await i.opSSeqAsync(sseq1);
                        test(opSSeqResult.ReturnValue[0].Equals(s1));
                        test(opSSeqResult.s2[0].Equals(s1));

                        Dictionary<String, Test.Inner.Inner2.S> smap1 = new Dictionary<String, Test.Inner.Inner2.S>();
                        smap1["a"] = s1;
                        var opSMapResult = await i.opSMapAsync(smap1);
                        test(opSMapResult.ReturnValue["a"].Equals(s1));
                        test(opSMapResult.s2["a"].Equals(s1));

                        Test.Inner.Inner2.C c1 = new Test.Inner.Inner2.C(s1);
                        var opCResult = await i.opCAsync(c1);
                        test(c1.s.Equals(opCResult.ReturnValue.s));
                        test(c1.s.Equals(opCResult.c2.s));

                        Test.Inner.Inner2.C[] cseq1 = new Test.Inner.Inner2.C[] { c1 };
                        var opCSeqResult = await i.opCSeqAsync(cseq1);
                        test(opCSeqResult.ReturnValue[0].s.Equals(s1));
                        test(opCSeqResult.c2[0].s.Equals(s1));

                        Dictionary<String, Test.Inner.Inner2.C> cmap1 = new Dictionary<String, Test.Inner.Inner2.C>();
                        cmap1["a"] = c1;
                        var opCMapResult = await i.opCMapAsync(cmap1);
                        test(opCMapResult.ReturnValue["a"].s.Equals(s1));
                        test(opCMapResult.c2["a"].s.Equals(s1));
                    }).Wait();
            }

            {
                var obj = IObjectPrx.Parse($"i4:{helper.getTestEndpoint()}", communicator);
                Inner.Test.Inner2.IIPrx i = Inner.Test.Inner2.IIPrx.CheckedCast(obj);

                Test.S s1 = new Test.S(0);
                var (s3, s2) = i.opS(s1);
                test(s2.Equals(s1));
                test(s3.Equals(s1));

                var sseq1 = new Test.S[] { s1 };
                var (sseq3, sseq2) = i.opSSeq(sseq1);
                test(sseq2[0].Equals(s1));
                test(sseq3[0].Equals(s1));

                var smap1 = new Dictionary<string, Test.S>();
                smap1["a"] = s1;
                var (smap3, smap2) = i.opSMap(smap1);
                test(smap2["a"].Equals(s1));
                test(smap3["a"].Equals(s1));

                Test.C c1 = new Test.C(s1);
                var (c3, c2) = i.opC(c1);
                test(c2.s.Equals(c1.s));
                test(c3.s.Equals(c1.s));

                var cseq1 = new Test.C[] { c1 };
                var (cseq3, cseq2) = i.opCSeq(cseq1);
                test(cseq2[0].s.Equals(s1));
                test(cseq3[0].s.Equals(s1));

                var cmap1 = new Dictionary<string, Test.C>();
                cmap1["a"] = c1;
                var (cmap3, cmap2) = i.opCMap(cmap1);
                test(cmap2["a"].s.Equals(s1));
                test(cmap3["a"].s.Equals(s1));
            }

            {
                var obj = IObjectPrx.Parse($"i4:{helper.getTestEndpoint()}", communicator);
                Inner.Test.Inner2.IIPrx i = Inner.Test.Inner2.IIPrx.CheckedCast(obj);

                Task.Run(async () =>
                    {
                        Test.S s1 = new Test.S(0);
                        var opSResult = await i.opSAsync(s1);
                        test(s1.Equals(opSResult.ReturnValue));
                        test(s1.Equals(opSResult.s2));

                        Test.S[] sseq1 = new Test.S[] { s1 };
                        var opSSeqResult = await i.opSSeqAsync(sseq1);
                        test(opSSeqResult.ReturnValue[0].Equals(s1));
                        test(opSSeqResult.s2[0].Equals(s1));

                        Dictionary<String, Test.S> smap1 = new Dictionary<String, Test.S>();
                        smap1["a"] = s1;
                        var opSMapResult = await i.opSMapAsync(smap1);
                        test(opSMapResult.ReturnValue["a"].Equals(s1));
                        test(opSMapResult.s2["a"].Equals(s1));

                        Test.C c1 = new Test.C(s1);
                        var opCResult = await i.opCAsync(c1);
                        test(c1.s.Equals(opCResult.ReturnValue.s));
                        test(c1.s.Equals(opCResult.c2.s));

                        Test.C[] cseq1 = new Test.C[] { c1 };
                        var opCSeqResult = await i.opCSeqAsync(cseq1);
                        test(opCSeqResult.ReturnValue[0].s.Equals(s1));
                        test(opCSeqResult.c2[0].s.Equals(s1));

                        Dictionary<String, Test.C> cmap1 = new Dictionary<String, Test.C>();
                        cmap1["a"] = c1;
                        var opCMapResult = await i.opCMapAsync(cmap1);
                        test(opCMapResult.ReturnValue["a"].s.Equals(s1));
                        test(opCMapResult.c2["a"].s.Equals(s1));
                    }).Wait();
            }

            {
                Test.IIPrx.Parse($"i1:{helper.getTestEndpoint()}", communicator).shutdown();
            }
        }
    }
}
