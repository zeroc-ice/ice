//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using System.Diagnostics;
using Test;

namespace Ice.scope
{
    public class AllTests
    {
        public static void allTests(TestHelper helper)
        {
            Communicator? communicator = helper.Communicator();
            TestHelper.Assert(communicator != null);
            {
                var i = Test.IIPrx.Parse($"i1:{helper.GetTestEndpoint()}", communicator);
                var s1 = new Test.S(0);
                var (s3, s2) = i.opS(s1);
                TestHelper.Assert(s2.Equals(s1));
                TestHelper.Assert(s3.Equals(s1));

                var sseq1 = new Test.S[] { s1 };
                var (sseq3, sseq2) = i.opSSeq(sseq1);
                TestHelper.Assert(sseq2[0].Equals(s1));
                TestHelper.Assert(sseq3[0].Equals(s1));

                Dictionary<string, Test.S> smap1 = new Dictionary<string, Test.S>();
                smap1["a"] = s1;
                var (smap3, smap2) = i.opSMap(smap1);
                TestHelper.Assert(smap2["a"].Equals(s1));
                TestHelper.Assert(smap3["a"].Equals(s1));

                var c1 = new Test.C(s1);
                var (c3, c2) = i.opC(c1);
                TestHelper.Assert(c2 != null && c3 != null);
                TestHelper.Assert(c2 != null && c2.s.Equals(c1.s));
                TestHelper.Assert(c3 != null && c3.s.Equals(c1.s));

                var cseq1 = new Test.C[] { c1 };
                var (cseq3, cseq2) = i.opCSeq(cseq1);
                TestHelper.Assert(cseq2[0]!.s.Equals(s1));
                TestHelper.Assert(cseq3[0]!.s.Equals(s1));

                var cmap1 = new Dictionary<string, Test.C?>();
                cmap1["a"] = c1;
                var (cmap3, cmap2) = i.opCMap(cmap1);
                TestHelper.Assert(cmap2["a"]!.s.Equals(s1));
                TestHelper.Assert(cmap3["a"]!.s.Equals(s1));

                var e = i.opE1(Test.E1.v1);
                TestHelper.Assert(e == Test.E1.v1);

                var s = i.opS1(new Test.S1("S1"));
                TestHelper.Assert(s.s == "S1");

                var c = i.opC1(new Test.C1("C1"));
                TestHelper.Assert(c != null && c.s == "C1");
            }

            {
                var i = Test.IIPrx.Parse($"i1:{helper.GetTestEndpoint()}", communicator);

                Task.Run(async () =>
                    {
                        var s1 = new Test.S(0);
                        var opSResult = await i.opSAsync(s1);
                        TestHelper.Assert(s1.Equals(opSResult.ReturnValue));
                        TestHelper.Assert(s1.Equals(opSResult.s2));

                        var sseq1 = new Test.S[] { s1 };
                        var opSSeqResult = await i.opSSeqAsync(sseq1);
                        TestHelper.Assert(opSSeqResult.ReturnValue[0].Equals(s1));
                        TestHelper.Assert(opSSeqResult.s2[0].Equals(s1));

                        var smap1 = new Dictionary<string, Test.S>();
                        smap1["a"] = s1;
                        var opSMapResult = await i.opSMapAsync(smap1);
                        TestHelper.Assert(opSMapResult.ReturnValue["a"].Equals(s1));
                        TestHelper.Assert(opSMapResult.s2["a"].Equals(s1));

                        var c1 = new Test.C(s1);
                        (Test.C? ReturnValue, Test.C? c2) opCResult = await i.opCAsync(c1);
                        TestHelper.Assert(opCResult.ReturnValue != null && c1.s.Equals(opCResult.ReturnValue.s));
                        TestHelper.Assert(opCResult.c2 != null && c1.s.Equals(opCResult.c2.s));

                        var cseq1 = new Test.C[] { c1 };
                        (Test.C?[] ReturnValue, Test.C?[] c2) opCSeqResult = await i.opCSeqAsync(cseq1);
                        TestHelper.Assert(opCSeqResult.ReturnValue[0]!.s.Equals(s1));
                        TestHelper.Assert(opCSeqResult.c2[0]!.s.Equals(s1));

                        var cmap1 = new Dictionary<string, Test.C?>
                        {
                            ["a"] = c1
                        };
                        (Dictionary<string, Test.C?> ReturnValue, Dictionary<string, Test.C?> c2) =
                            await i.opCMapAsync(cmap1);
                        TestHelper.Assert(ReturnValue["a"]!.s.Equals(s1));
                        TestHelper.Assert(c2["a"]!.s.Equals(s1));

                        Test.E1 e = await i.opE1Async(Test.E1.v1);
                        TestHelper.Assert(e == Test.E1.v1);

                        Test.S1 s = await i.opS1Async(new Test.S1("S1"));
                        TestHelper.Assert(s.s == "S1");

                        Test.C1? c = await i.opC1Async(new Test.C1("C1"));
                        TestHelper.Assert(c != null && c.s == "C1");
                    }).Wait();
            }

            {
                var i = Test.Inner.IIPrx.Parse($"i2:{helper.GetTestEndpoint()}", communicator);

                Test.Inner.Inner2.S s1 = new Test.Inner.Inner2.S(0);
                var (s3, s2) = i.opS(s1);
                TestHelper.Assert(s2.Equals(s1));
                TestHelper.Assert(s3.Equals(s1));

                Test.Inner.Inner2.S[] sseq1 = new Test.Inner.Inner2.S[] { s1 };
                var (sseq3, sseq2) = i.opSSeq(sseq1);
                TestHelper.Assert(sseq2[0].Equals(s1));
                TestHelper.Assert(sseq3[0].Equals(s1));

                var smap1 = new Dictionary<string, Test.Inner.Inner2.S>();
                smap1["a"] = s1;
                var (smap3, smap2) = i.opSMap(smap1);
                TestHelper.Assert(smap2["a"].Equals(s1));
                TestHelper.Assert(smap3["a"].Equals(s1));

                var c1 = new Test.Inner.Inner2.C(s1);
                var (c3, c2) = i.opC(c1);
                TestHelper.Assert(c2!.s.Equals(c1.s));
                TestHelper.Assert(c3!.s.Equals(c1.s));

                var cseq1 = new Test.Inner.Inner2.C[] { c1 };
                var (cseq3, cseq2) = i.opCSeq(cseq1);
                TestHelper.Assert(cseq2[0]!.s.Equals(s1));
                TestHelper.Assert(cseq3[0]!.s.Equals(s1));

                var cmap1 = new Dictionary<string, Test.Inner.Inner2.C?> { ["a"] = c1 };
                var (cmap3, cmap2) = i.opCMap(cmap1);
                TestHelper.Assert(cmap2["a"]!.s.Equals(s1));
                TestHelper.Assert(cmap3["a"]!.s.Equals(s1));
            }

            {
                var i = Test.Inner.IIPrx.Parse($"i2:{helper.GetTestEndpoint()}", communicator);
                Task.Run(async () =>
                    {
                        Test.Inner.Inner2.S s1 = new Test.Inner.Inner2.S(0);
                        var opSResult = await i.opSAsync(s1);
                        TestHelper.Assert(s1.Equals(opSResult.ReturnValue));
                        TestHelper.Assert(s1.Equals(opSResult.s2));

                        var sseq1 = new Test.Inner.Inner2.S[] { s1 };
                        (Test.Inner.Inner2.S[] ReturnValue,
                         Test.Inner.Inner2.S[] s2) opSSeqResult = await i.opSSeqAsync(sseq1);
                        TestHelper.Assert(opSSeqResult.ReturnValue[0].Equals(s1));
                        TestHelper.Assert(opSSeqResult.s2[0].Equals(s1));

                        var smap1 = new Dictionary<string, Test.Inner.Inner2.S> { ["a"] = s1 };
                        (Dictionary<string, Test.Inner.Inner2.S> ReturnValue,
                         Dictionary<string, Test.Inner.Inner2.S> s2) = await i.opSMapAsync(smap1);
                        TestHelper.Assert(ReturnValue["a"].Equals(s1));
                        TestHelper.Assert(s2["a"].Equals(s1));

                        var c1 = new Test.Inner.Inner2.C(s1);
                        (Test.Inner.Inner2.C? ReturnValue, Test.Inner.Inner2.C? c2) opCResult = await i.opCAsync(c1);
                        TestHelper.Assert(c1.s.Equals(opCResult.ReturnValue!.s));
                        TestHelper.Assert(c1.s.Equals(opCResult.c2!.s));

                        Test.Inner.Inner2.C[] cseq1 = new Test.Inner.Inner2.C[] { c1 };
                        (Test.Inner.Inner2.C?[] ReturnValue,
                         Test.Inner.Inner2.C?[] c2) opCSeqResult = await i.opCSeqAsync(cseq1);
                        TestHelper.Assert(opCSeqResult.ReturnValue[0]!.s.Equals(s1));
                        TestHelper.Assert(opCSeqResult.c2[0]!.s.Equals(s1));

                        var cmap1 = new Dictionary<string, Test.Inner.Inner2.C?> { ["a"] = c1 };
                        (Dictionary<string, Test.Inner.Inner2.C?> ReturnValue,
                         Dictionary<string, Test.Inner.Inner2.C?> c2) opCMapResult = await i.opCMapAsync(cmap1);
                        TestHelper.Assert(opCMapResult.ReturnValue["a"]!.s.Equals(s1));
                        TestHelper.Assert(opCMapResult.c2["a"]!.s.Equals(s1));
                    }).Wait();
            }

            {
                var i = Test.Inner.Inner2.IIPrx.Parse($"i3:{helper.GetTestEndpoint()}", communicator);

                var s1 = new Test.Inner.Inner2.S(0);
                var (s3, s2) = i.opS(s1);
                TestHelper.Assert(s2.Equals(s1));
                TestHelper.Assert(s3.Equals(s1));

                var sseq1 = new Test.Inner.Inner2.S[] { s1 };
                var (sseq3, sseq2) = i.opSSeq(sseq1);
                TestHelper.Assert(sseq2[0].Equals(s1));
                TestHelper.Assert(sseq3[0].Equals(s1));

                var smap1 = new Dictionary<string, Test.Inner.Inner2.S>();
                smap1["a"] = s1;
                var (smap3, smap2) = i.opSMap(smap1);
                TestHelper.Assert(smap2["a"].Equals(s1));
                TestHelper.Assert(smap3["a"].Equals(s1));

                var c1 = new Test.Inner.Inner2.C(s1);
                var (c3, c2) = i.opC(c1);
                TestHelper.Assert(c2!.s.Equals(c1.s));
                TestHelper.Assert(c3!.s.Equals(c1.s));

                var cseq1 = new Test.Inner.Inner2.C[] { c1 };
                var (cseq3, cseq2) = i.opCSeq(cseq1);
                TestHelper.Assert(cseq2[0]!.s.Equals(s1));
                TestHelper.Assert(cseq3[0]!.s.Equals(s1));

                var cmap1 = new Dictionary<string, Test.Inner.Inner2.C?> { ["a"] = c1 };
                (Dictionary<string, Test.Inner.Inner2.C?> cmap3,
                 Dictionary<string, Test.Inner.Inner2.C?> cmap2) = i.opCMap(cmap1);
                TestHelper.Assert(cmap2["a"]!.s.Equals(s1));
                TestHelper.Assert(cmap3["a"]!.s.Equals(s1));
            }

            {
                var i = Test.Inner.Inner2.IIPrx.Parse($"i3:{helper.GetTestEndpoint()}", communicator);

                Task.Run(async () =>
                    {
                        var s1 = new Test.Inner.Inner2.S(0);
                        var opSResult = await i.opSAsync(s1);
                        TestHelper.Assert(s1.Equals(opSResult.ReturnValue));
                        TestHelper.Assert(s1.Equals(opSResult.s2));

                        var sseq1 = new Test.Inner.Inner2.S[] { s1 };
                        var opSSeqResult = await i.opSSeqAsync(sseq1);
                        TestHelper.Assert(opSSeqResult.ReturnValue[0].Equals(s1));
                        TestHelper.Assert(opSSeqResult.s2[0].Equals(s1));

                        var smap1 = new Dictionary<string, Test.Inner.Inner2.S> { ["a"] = s1 };
                        var opSMapResult = await i.opSMapAsync(smap1);
                        TestHelper.Assert(opSMapResult.ReturnValue["a"].Equals(s1));
                        TestHelper.Assert(opSMapResult.s2["a"].Equals(s1));

                        Test.Inner.Inner2.C c1 = new Test.Inner.Inner2.C(s1);
                        var opCResult = await i.opCAsync(c1);
                        TestHelper.Assert(c1.s.Equals(opCResult.ReturnValue!.s));
                        TestHelper.Assert(c1.s.Equals(opCResult.c2!.s));

                        Test.Inner.Inner2.C[] cseq1 = new Test.Inner.Inner2.C[] { c1 };
                        var opCSeqResult = await i.opCSeqAsync(cseq1);
                        TestHelper.Assert(opCSeqResult.ReturnValue[0]!.s.Equals(s1));
                        TestHelper.Assert(opCSeqResult.c2[0]!.s.Equals(s1));

                        var cmap1 = new Dictionary<string, Test.Inner.Inner2.C?> { ["a"] = c1 };
                        var opCMapResult = await i.opCMapAsync(cmap1);
                        TestHelper.Assert(opCMapResult.ReturnValue["a"]!.s.Equals(s1));
                        TestHelper.Assert(opCMapResult.c2["a"]!.s.Equals(s1));
                    }).Wait();
            }

            {
                var i = Inner.Test.Inner2.IIPrx.Parse($"i4:{helper.GetTestEndpoint()}", communicator);

                Test.S s1 = new Test.S(0);
                var (s3, s2) = i.opS(s1);
                TestHelper.Assert(s2.Equals(s1));
                TestHelper.Assert(s3.Equals(s1));

                var sseq1 = new Test.S[] { s1 };
                var (sseq3, sseq2) = i.opSSeq(sseq1);
                TestHelper.Assert(sseq2[0].Equals(s1));
                TestHelper.Assert(sseq3[0].Equals(s1));

                var smap1 = new Dictionary<string, Test.S>();
                smap1["a"] = s1;
                var (smap3, smap2) = i.opSMap(smap1);
                TestHelper.Assert(smap2["a"].Equals(s1));
                TestHelper.Assert(smap3["a"].Equals(s1));

                Test.C c1 = new Test.C(s1);
                var (c3, c2) = i.opC(c1);
                TestHelper.Assert(c2!.s.Equals(c1.s));
                TestHelper.Assert(c3!.s.Equals(c1.s));

                var cseq1 = new Test.C[] { c1 };
                var (cseq3, cseq2) = i.opCSeq(cseq1);
                TestHelper.Assert(cseq2[0]!.s.Equals(s1));
                TestHelper.Assert(cseq3[0]!.s.Equals(s1));

                var cmap1 = new Dictionary<string, Test.C?> { ["a"] = c1 };
                var (cmap3, cmap2) = i.opCMap(cmap1);
                TestHelper.Assert(cmap2["a"]!.s.Equals(s1));
                TestHelper.Assert(cmap3["a"]!.s.Equals(s1));
            }

            {
                var i = Inner.Test.Inner2.IIPrx.Parse($"i4:{helper.GetTestEndpoint()}", communicator);

                Task.Run(async () =>
                    {
                        Test.S s1 = new Test.S(0);
                        var opSResult = await i.opSAsync(s1);
                        TestHelper.Assert(s1.Equals(opSResult.ReturnValue));
                        TestHelper.Assert(s1.Equals(opSResult.s2));

                        var sseq1 = new Test.S[] { s1 };
                        var opSSeqResult = await i.opSSeqAsync(sseq1);
                        TestHelper.Assert(opSSeqResult.ReturnValue[0].Equals(s1));
                        TestHelper.Assert(opSSeqResult.s2[0].Equals(s1));

                        var smap1 = new Dictionary<string, Test.S> { ["a"] = s1 };
                        var opSMapResult = await i.opSMapAsync(smap1);
                        TestHelper.Assert(opSMapResult.ReturnValue["a"].Equals(s1));
                        TestHelper.Assert(opSMapResult.s2["a"].Equals(s1));

                        Test.C c1 = new Test.C(s1);
                        var opCResult = await i.opCAsync(c1);
                        TestHelper.Assert(c1.s.Equals(opCResult.ReturnValue!.s));
                        TestHelper.Assert(c1.s.Equals(opCResult.c2!.s));

                        Test.C[] cseq1 = new Test.C[] { c1 };
                        var opCSeqResult = await i.opCSeqAsync(cseq1);
                        TestHelper.Assert(opCSeqResult.ReturnValue[0]!.s.Equals(s1));
                        TestHelper.Assert(opCSeqResult.c2[0]!.s.Equals(s1));

                        var cmap1 = new Dictionary<string, Test.C?>();
                        cmap1["a"] = c1;
                        var opCMapResult = await i.opCMapAsync(cmap1);
                        TestHelper.Assert(opCMapResult.ReturnValue["a"]!.s.Equals(s1));
                        TestHelper.Assert(opCMapResult.c2["a"]!.s.Equals(s1));
                    }).Wait();
            }

            {
                Test.IIPrx.Parse($"i1:{helper.GetTestEndpoint()}", communicator).shutdown();
            }
        }
    }
}
