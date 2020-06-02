//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.Scope
{
    public class AllTests
    {
        public static void allTests(TestHelper helper)
        {
            Communicator? communicator = helper.Communicator();
            TestHelper.Assert(communicator != null);
            {
                var i = IIPrx.Parse($"i1:{helper.GetTestEndpoint()}", communicator);
                var s1 = new S(0);
                var (s3, s2) = i.opS(s1);
                TestHelper.Assert(s2.Equals(s1));
                TestHelper.Assert(s3.Equals(s1));

                var sseq1 = new S[] { s1 };
                var (sseq3, sseq2) = i.opSSeq(sseq1);
                TestHelper.Assert(sseq2[0].Equals(s1));
                TestHelper.Assert(sseq3[0].Equals(s1));

                Dictionary<string, S> smap1 = new Dictionary<string, S>();
                smap1["a"] = s1;
                var (smap3, smap2) = i.opSMap(smap1);
                TestHelper.Assert(smap2["a"].Equals(s1));
                TestHelper.Assert(smap3["a"].Equals(s1));

                var c1 = new C(s1);
                var (c3, c2) = i.opC(c1);
                TestHelper.Assert(c2 != null && c3 != null);
                TestHelper.Assert(c2 != null && c2.s.Equals(c1.s));
                TestHelper.Assert(c3 != null && c3.s.Equals(c1.s));

                var cseq1 = new C[] { c1 };
                var (cseq3, cseq2) = i.opCSeq(cseq1);
                TestHelper.Assert(cseq2[0]!.s.Equals(s1));
                TestHelper.Assert(cseq3[0]!.s.Equals(s1));

                var cmap1 = new Dictionary<string, C?>();
                cmap1["a"] = c1;
                var (cmap3, cmap2) = i.opCMap(cmap1);
                TestHelper.Assert(cmap2["a"]!.s.Equals(s1));
                TestHelper.Assert(cmap3["a"]!.s.Equals(s1));

                var e = i.opE1(E1.v1);
                TestHelper.Assert(e == E1.v1);

                var s = i.opS1(new S1("S1"));
                TestHelper.Assert(s.s == "S1");

                var c = i.opC1(new C1("C1"));
                TestHelper.Assert(c != null && c.s == "C1");
            }

            {
                var i = IIPrx.Parse($"i1:{helper.GetTestEndpoint()}", communicator);

                Task.Run(async () =>
                    {
                        var s1 = new S(0);
                        var opSResult = await i.opSAsync(s1);
                        TestHelper.Assert(s1.Equals(opSResult.ReturnValue));
                        TestHelper.Assert(s1.Equals(opSResult.s2));

                        var sseq1 = new S[] { s1 };
                        var opSSeqResult = await i.opSSeqAsync(sseq1);
                        TestHelper.Assert(opSSeqResult.ReturnValue[0].Equals(s1));
                        TestHelper.Assert(opSSeqResult.s2[0].Equals(s1));

                        var smap1 = new Dictionary<string, S>();
                        smap1["a"] = s1;
                        var opSMapResult = await i.opSMapAsync(smap1);
                        TestHelper.Assert(opSMapResult.ReturnValue["a"].Equals(s1));
                        TestHelper.Assert(opSMapResult.s2["a"].Equals(s1));

                        var c1 = new C(s1);
                        (C? ReturnValue, C? c2) opCResult = await i.opCAsync(c1);
                        TestHelper.Assert(opCResult.ReturnValue != null && c1.s.Equals(opCResult.ReturnValue.s));
                        TestHelper.Assert(opCResult.c2 != null && c1.s.Equals(opCResult.c2.s));

                        var cseq1 = new C[] { c1 };
                        (C?[] ReturnValue, C?[] c2) opCSeqResult = await i.opCSeqAsync(cseq1);
                        TestHelper.Assert(opCSeqResult.ReturnValue[0]!.s.Equals(s1));
                        TestHelper.Assert(opCSeqResult.c2[0]!.s.Equals(s1));

                        var cmap1 = new Dictionary<string, C?>
                        {
                            ["a"] = c1
                        };
                        (Dictionary<string, C?> ReturnValue, Dictionary<string, C?> c2) =
                            await i.opCMapAsync(cmap1);
                        TestHelper.Assert(ReturnValue["a"]!.s.Equals(s1));
                        TestHelper.Assert(c2["a"]!.s.Equals(s1));

                        E1 e = await i.opE1Async(E1.v1);
                        TestHelper.Assert(e == E1.v1);

                        S1 s = await i.opS1Async(new S1("S1"));
                        TestHelper.Assert(s.s == "S1");

                        C1? c = await i.opC1Async(new C1("C1"));
                        TestHelper.Assert(c != null && c.s == "C1");
                    }).Wait();
            }

            {
                var i = Inner.IIPrx.Parse($"i2:{helper.GetTestEndpoint()}", communicator);

                Inner.Inner2.S s1 = new Inner.Inner2.S(0);
                var (s3, s2) = i.opS(s1);
                TestHelper.Assert(s2.Equals(s1));
                TestHelper.Assert(s3.Equals(s1));

                Inner.Inner2.S[] sseq1 = new Inner.Inner2.S[] { s1 };
                var (sseq3, sseq2) = i.opSSeq(sseq1);
                TestHelper.Assert(sseq2[0].Equals(s1));
                TestHelper.Assert(sseq3[0].Equals(s1));

                var smap1 = new Dictionary<string, Inner.Inner2.S>();
                smap1["a"] = s1;
                var (smap3, smap2) = i.opSMap(smap1);
                TestHelper.Assert(smap2["a"].Equals(s1));
                TestHelper.Assert(smap3["a"].Equals(s1));

                var c1 = new Inner.Inner2.C(s1);
                var (c3, c2) = i.opC(c1);
                TestHelper.Assert(c2!.s.Equals(c1.s));
                TestHelper.Assert(c3!.s.Equals(c1.s));

                var cseq1 = new Inner.Inner2.C[] { c1 };
                var (cseq3, cseq2) = i.opCSeq(cseq1);
                TestHelper.Assert(cseq2[0]!.s.Equals(s1));
                TestHelper.Assert(cseq3[0]!.s.Equals(s1));

                var cmap1 = new Dictionary<string, Inner.Inner2.C?> { ["a"] = c1 };
                var (cmap3, cmap2) = i.opCMap(cmap1);
                TestHelper.Assert(cmap2["a"]!.s.Equals(s1));
                TestHelper.Assert(cmap3["a"]!.s.Equals(s1));
            }

            {
                var i = Inner.IIPrx.Parse($"i2:{helper.GetTestEndpoint()}", communicator);
                Task.Run(async () =>
                    {
                        Inner.Inner2.S s1 = new Inner.Inner2.S(0);
                        var opSResult = await i.opSAsync(s1);
                        TestHelper.Assert(s1.Equals(opSResult.ReturnValue));
                        TestHelper.Assert(s1.Equals(opSResult.s2));

                        var sseq1 = new Inner.Inner2.S[] { s1 };
                        (Inner.Inner2.S[] ReturnValue, Inner.Inner2.S[] s2) opSSeqResult = await i.opSSeqAsync(sseq1);
                        TestHelper.Assert(opSSeqResult.ReturnValue[0].Equals(s1));
                        TestHelper.Assert(opSSeqResult.s2[0].Equals(s1));

                        var smap1 = new Dictionary<string, Inner.Inner2.S> { ["a"] = s1 };
                        (Dictionary<string, Inner.Inner2.S> ReturnValue,
                         Dictionary<string, Inner.Inner2.S> s2) = await i.opSMapAsync(smap1);
                        TestHelper.Assert(ReturnValue["a"].Equals(s1));
                        TestHelper.Assert(s2["a"].Equals(s1));

                        var c1 = new Inner.Inner2.C(s1);
                        (Inner.Inner2.C? ReturnValue, Inner.Inner2.C? c2) opCResult = await i.opCAsync(c1);
                        TestHelper.Assert(c1.s.Equals(opCResult.ReturnValue!.s));
                        TestHelper.Assert(c1.s.Equals(opCResult.c2!.s));

                        Inner.Inner2.C[] cseq1 = new Inner.Inner2.C[] { c1 };
                        (Inner.Inner2.C?[] ReturnValue, Inner.Inner2.C?[] c2) opCSeqResult = await i.opCSeqAsync(cseq1);
                        TestHelper.Assert(opCSeqResult.ReturnValue[0]!.s.Equals(s1));
                        TestHelper.Assert(opCSeqResult.c2[0]!.s.Equals(s1));

                        var cmap1 = new Dictionary<string, Inner.Inner2.C?> { ["a"] = c1 };
                        (Dictionary<string, Inner.Inner2.C?> ReturnValue,
                         Dictionary<string, Inner.Inner2.C?> c2) opCMapResult = await i.opCMapAsync(cmap1);
                        TestHelper.Assert(opCMapResult.ReturnValue["a"]!.s.Equals(s1));
                        TestHelper.Assert(opCMapResult.c2["a"]!.s.Equals(s1));
                    }).Wait();
            }

            {
                var i = Inner.Inner2.IIPrx.Parse($"i3:{helper.GetTestEndpoint()}", communicator);

                var s1 = new Inner.Inner2.S(0);
                var (s3, s2) = i.opS(s1);
                TestHelper.Assert(s2.Equals(s1));
                TestHelper.Assert(s3.Equals(s1));

                var sseq1 = new Inner.Inner2.S[] { s1 };
                var (sseq3, sseq2) = i.opSSeq(sseq1);
                TestHelper.Assert(sseq2[0].Equals(s1));
                TestHelper.Assert(sseq3[0].Equals(s1));

                var smap1 = new Dictionary<string, Inner.Inner2.S>();
                smap1["a"] = s1;
                var (smap3, smap2) = i.opSMap(smap1);
                TestHelper.Assert(smap2["a"].Equals(s1));
                TestHelper.Assert(smap3["a"].Equals(s1));

                var c1 = new Inner.Inner2.C(s1);
                var (c3, c2) = i.opC(c1);
                TestHelper.Assert(c2!.s.Equals(c1.s));
                TestHelper.Assert(c3!.s.Equals(c1.s));

                var cseq1 = new Inner.Inner2.C[] { c1 };
                var (cseq3, cseq2) = i.opCSeq(cseq1);
                TestHelper.Assert(cseq2[0]!.s.Equals(s1));
                TestHelper.Assert(cseq3[0]!.s.Equals(s1));

                var cmap1 = new Dictionary<string, Inner.Inner2.C?> { ["a"] = c1 };
                (Dictionary<string, Inner.Inner2.C?> cmap3,
                 Dictionary<string, Inner.Inner2.C?> cmap2) = i.opCMap(cmap1);
                TestHelper.Assert(cmap2["a"]!.s.Equals(s1));
                TestHelper.Assert(cmap3["a"]!.s.Equals(s1));
            }

            {
                var i = Inner.Inner2.IIPrx.Parse($"i3:{helper.GetTestEndpoint()}", communicator);

                Task.Run(async () =>
                    {
                        var s1 = new Inner.Inner2.S(0);
                        var opSResult = await i.opSAsync(s1);
                        TestHelper.Assert(s1.Equals(opSResult.ReturnValue));
                        TestHelper.Assert(s1.Equals(opSResult.s2));

                        var sseq1 = new Inner.Inner2.S[] { s1 };
                        var opSSeqResult = await i.opSSeqAsync(sseq1);
                        TestHelper.Assert(opSSeqResult.ReturnValue[0].Equals(s1));
                        TestHelper.Assert(opSSeqResult.s2[0].Equals(s1));

                        var smap1 = new Dictionary<string, Inner.Inner2.S> { ["a"] = s1 };
                        var opSMapResult = await i.opSMapAsync(smap1);
                        TestHelper.Assert(opSMapResult.ReturnValue["a"].Equals(s1));
                        TestHelper.Assert(opSMapResult.s2["a"].Equals(s1));

                        Inner.Inner2.C c1 = new Inner.Inner2.C(s1);
                        var opCResult = await i.opCAsync(c1);
                        TestHelper.Assert(c1.s.Equals(opCResult.ReturnValue!.s));
                        TestHelper.Assert(c1.s.Equals(opCResult.c2!.s));

                        Inner.Inner2.C[] cseq1 = new Inner.Inner2.C[] { c1 };
                        var opCSeqResult = await i.opCSeqAsync(cseq1);
                        TestHelper.Assert(opCSeqResult.ReturnValue[0]!.s.Equals(s1));
                        TestHelper.Assert(opCSeqResult.c2[0]!.s.Equals(s1));

                        var cmap1 = new Dictionary<string, Inner.Inner2.C?> { ["a"] = c1 };
                        var opCMapResult = await i.opCMapAsync(cmap1);
                        TestHelper.Assert(opCMapResult.ReturnValue["a"]!.s.Equals(s1));
                        TestHelper.Assert(opCMapResult.c2["a"]!.s.Equals(s1));
                    }).Wait();
            }

            {
                var i = Inner.Test.Inner2.IIPrx.Parse($"i4:{helper.GetTestEndpoint()}", communicator);

                S s1 = new S(0);
                var (s3, s2) = i.opS(s1);
                TestHelper.Assert(s2.Equals(s1));
                TestHelper.Assert(s3.Equals(s1));

                var sseq1 = new S[] { s1 };
                var (sseq3, sseq2) = i.opSSeq(sseq1);
                TestHelper.Assert(sseq2[0].Equals(s1));
                TestHelper.Assert(sseq3[0].Equals(s1));

                var smap1 = new Dictionary<string, S>();
                smap1["a"] = s1;
                var (smap3, smap2) = i.opSMap(smap1);
                TestHelper.Assert(smap2["a"].Equals(s1));
                TestHelper.Assert(smap3["a"].Equals(s1));

                C c1 = new C(s1);
                var (c3, c2) = i.opC(c1);
                TestHelper.Assert(c2!.s.Equals(c1.s));
                TestHelper.Assert(c3!.s.Equals(c1.s));

                var cseq1 = new C[] { c1 };
                var (cseq3, cseq2) = i.opCSeq(cseq1);
                TestHelper.Assert(cseq2[0]!.s.Equals(s1));
                TestHelper.Assert(cseq3[0]!.s.Equals(s1));

                var cmap1 = new Dictionary<string, C?> { ["a"] = c1 };
                var (cmap3, cmap2) = i.opCMap(cmap1);
                TestHelper.Assert(cmap2["a"]!.s.Equals(s1));
                TestHelper.Assert(cmap3["a"]!.s.Equals(s1));
            }

            {
                var i = Inner.Test.Inner2.IIPrx.Parse($"i4:{helper.GetTestEndpoint()}", communicator);

                Task.Run(async () =>
                    {
                        S s1 = new S(0);
                        var opSResult = await i.opSAsync(s1);
                        TestHelper.Assert(s1.Equals(opSResult.ReturnValue));
                        TestHelper.Assert(s1.Equals(opSResult.s2));

                        var sseq1 = new S[] { s1 };
                        var opSSeqResult = await i.opSSeqAsync(sseq1);
                        TestHelper.Assert(opSSeqResult.ReturnValue[0].Equals(s1));
                        TestHelper.Assert(opSSeqResult.s2[0].Equals(s1));

                        var smap1 = new Dictionary<string, S> { ["a"] = s1 };
                        var opSMapResult = await i.opSMapAsync(smap1);
                        TestHelper.Assert(opSMapResult.ReturnValue["a"].Equals(s1));
                        TestHelper.Assert(opSMapResult.s2["a"].Equals(s1));

                        C c1 = new C(s1);
                        var opCResult = await i.opCAsync(c1);
                        TestHelper.Assert(c1.s.Equals(opCResult.ReturnValue!.s));
                        TestHelper.Assert(c1.s.Equals(opCResult.c2!.s));

                        C[] cseq1 = new C[] { c1 };
                        var opCSeqResult = await i.opCSeqAsync(cseq1);
                        TestHelper.Assert(opCSeqResult.ReturnValue[0]!.s.Equals(s1));
                        TestHelper.Assert(opCSeqResult.c2[0]!.s.Equals(s1));

                        var cmap1 = new Dictionary<string, C?>();
                        cmap1["a"] = c1;
                        var opCMapResult = await i.opCMapAsync(cmap1);
                        TestHelper.Assert(opCMapResult.ReturnValue["a"]!.s.Equals(s1));
                        TestHelper.Assert(opCMapResult.c2["a"]!.s.Equals(s1));
                    }).Wait();
            }

            {
                IIPrx.Parse($"i1:{helper.GetTestEndpoint()}", communicator).shutdown();
            }
        }
    }
}
