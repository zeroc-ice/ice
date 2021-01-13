// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
using System.IO;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Scope
{
    public static class AllTests
    {
        public static Task RunAsync(TestHelper helper)
        {
            Communicator communicator = helper.Communicator;
            TextWriter output = helper.Output;

            output.Write("test using same type name in different Slice modules... ");
            output.Flush();
            {
                var i = IIPrx.Parse(helper.GetTestProxy("i1"), communicator);
                var s1 = new S(0);
                var (s3, s2) = i.OpS(s1);
                TestHelper.Assert(s2.Equals(s1));
                TestHelper.Assert(s3.Equals(s1));

                var sseq1 = new S[] { s1 };
                var (sseq3, sseq2) = i.OpSSeq(sseq1);
                TestHelper.Assert(sseq2[0].Equals(s1));
                TestHelper.Assert(sseq3[0].Equals(s1));

                var smap1 = new Dictionary<string, S>();
                smap1["a"] = s1;
                var (smap3, smap2) = i.OpSMap(smap1);
                TestHelper.Assert(smap2["a"].Equals(s1));
                TestHelper.Assert(smap3["a"].Equals(s1));

                var c1 = new C(s1);
                var (c3, c2) = i.OpC(c1);
                TestHelper.Assert(c2 != null && c3 != null);
                TestHelper.Assert(c2 != null && c2.S.Equals(c1.S));
                TestHelper.Assert(c3 != null && c3.S.Equals(c1.S));

                var cseq1 = new C[] { c1 };
                var (cseq3, cseq2) = i.OpCSeq(cseq1);
                TestHelper.Assert(cseq2[0]!.S.Equals(s1));
                TestHelper.Assert(cseq3[0]!.S.Equals(s1));

                var cmap1 = new Dictionary<string, C?>();
                cmap1["a"] = c1;
                var (cmap3, cmap2) = i.OpCMap(cmap1);
                TestHelper.Assert(cmap2["a"]!.S.Equals(s1));
                TestHelper.Assert(cmap3["a"]!.S.Equals(s1));

                var e = i.OpE1(E1.v1);
                TestHelper.Assert(e == E1.v1);

                var s = i.OpS1(new S1("S1"));
                TestHelper.Assert(s.S == "S1");

                var c = i.OpC1(new C1("C1"));
                TestHelper.Assert(c != null && c.S == "C1");
            }

            {
                var i = IIPrx.Parse(helper.GetTestProxy("i1"), communicator);

                Task.Run(async () =>
                    {
                        var s1 = new S(0);
                        var opSResult = await i.OpSAsync(s1);
                        TestHelper.Assert(s1.Equals(opSResult.R1));
                        TestHelper.Assert(s1.Equals(opSResult.R2));

                        var sseq1 = new S[] { s1 };
                        var opSSeqResult = await i.OpSSeqAsync(sseq1);
                        TestHelper.Assert(opSSeqResult.R1[0].Equals(s1));
                        TestHelper.Assert(opSSeqResult.R2[0].Equals(s1));

                        var smap1 = new Dictionary<string, S>();
                        smap1["a"] = s1;
                        var opSMapResult = await i.OpSMapAsync(smap1);
                        TestHelper.Assert(opSMapResult.R1["a"].Equals(s1));
                        TestHelper.Assert(opSMapResult.R2["a"].Equals(s1));

                        var c1 = new C(s1);
                        (C? ReturnValue, C? C2) opCResult = await i.OpCAsync(c1);
                        TestHelper.Assert(opCResult.ReturnValue != null && c1.S.Equals(opCResult.ReturnValue.S));
                        TestHelper.Assert(opCResult.C2 != null && c1.S.Equals(opCResult.C2.S));

                        var cseq1 = new C[] { c1 };
                        (C?[] ReturnValue, C?[] C2) opCSeqResult = await i.OpCSeqAsync(cseq1);
                        TestHelper.Assert(opCSeqResult.ReturnValue[0]!.S.Equals(s1));
                        TestHelper.Assert(opCSeqResult.C2[0]!.S.Equals(s1));

                        var cmap1 = new Dictionary<string, C?>
                        {
                            ["a"] = c1
                        };
                        (Dictionary<string, C?> returnValue, Dictionary<string, C?> c2) =
                            await i.OpCMapAsync(cmap1);
                        TestHelper.Assert(returnValue["a"]!.S.Equals(s1));
                        TestHelper.Assert(c2["a"]!.S.Equals(s1));

                        E1 e = await i.OpE1Async(E1.v1);
                        TestHelper.Assert(e == E1.v1);

                        S1 s = await i.OpS1Async(new S1("S1"));
                        TestHelper.Assert(s.S == "S1");

                        C1? c = await i.OpC1Async(new C1("C1"));
                        TestHelper.Assert(c != null && c.S == "C1");
                    }).Wait();
            }

            {
                var i = Inner.IIPrx.Parse(helper.GetTestProxy("i2"), communicator);

                var s1 = new Inner.Inner2.S(0);
                var (s3, s2) = i.OpS(s1);
                TestHelper.Assert(s2.Equals(s1));
                TestHelper.Assert(s3.Equals(s1));

                Inner.Inner2.S[] sseq1 = new Inner.Inner2.S[] { s1 };
                var (sseq3, sseq2) = i.OpSSeq(sseq1);
                TestHelper.Assert(sseq2[0].Equals(s1));
                TestHelper.Assert(sseq3[0].Equals(s1));

                var smap1 = new Dictionary<string, Inner.Inner2.S>();
                smap1["a"] = s1;
                var (smap3, smap2) = i.OpSMap(smap1);
                TestHelper.Assert(smap2["a"].Equals(s1));
                TestHelper.Assert(smap3["a"].Equals(s1));

                var c1 = new Inner.Inner2.C(s1);
                var (c3, c2) = i.OpC(c1);
                TestHelper.Assert(c2!.S.Equals(c1.S));
                TestHelper.Assert(c3!.S.Equals(c1.S));

                var cseq1 = new Inner.Inner2.C[] { c1 };
                var (cseq3, cseq2) = i.OpCSeq(cseq1);
                TestHelper.Assert(cseq2[0]!.S.Equals(s1));
                TestHelper.Assert(cseq3[0]!.S.Equals(s1));

                var cmap1 = new Dictionary<string, Inner.Inner2.C?> { ["a"] = c1 };
                var (cmap3, cmap2) = i.OpCMap(cmap1);
                TestHelper.Assert(cmap2["a"]!.S.Equals(s1));
                TestHelper.Assert(cmap3["a"]!.S.Equals(s1));
            }

            {
                var i = Inner.IIPrx.Parse(helper.GetTestProxy("i2"), communicator);
                Task.Run(async () =>
                    {
                        var s1 = new Inner.Inner2.S(0);
                        var opSResult = await i.OpSAsync(s1);
                        TestHelper.Assert(s1.Equals(opSResult.R1));
                        TestHelper.Assert(s1.Equals(opSResult.R2));

                        var sseq1 = new Inner.Inner2.S[] { s1 };
                        (Inner.Inner2.S[] ReturnValue, Inner.Inner2.S[] S2) opSSeqResult = await i.OpSSeqAsync(sseq1);
                        TestHelper.Assert(opSSeqResult.ReturnValue[0].Equals(s1));
                        TestHelper.Assert(opSSeqResult.S2[0].Equals(s1));

                        var smap1 = new Dictionary<string, Inner.Inner2.S> { ["a"] = s1 };
                        (Dictionary<string, Inner.Inner2.S> returnValue,
                         Dictionary<string, Inner.Inner2.S> s2) = await i.OpSMapAsync(smap1);
                        TestHelper.Assert(returnValue["a"].Equals(s1));
                        TestHelper.Assert(s2["a"].Equals(s1));

                        var c1 = new Inner.Inner2.C(s1);
                        (Inner.Inner2.C? ReturnValue, Inner.Inner2.C? C2) opCResult = await i.OpCAsync(c1);
                        TestHelper.Assert(c1.S.Equals(opCResult.ReturnValue!.S));
                        TestHelper.Assert(c1.S.Equals(opCResult.C2!.S));

                        Inner.Inner2.C[] cseq1 = new Inner.Inner2.C[] { c1 };
                        (Inner.Inner2.C?[] ReturnValue, Inner.Inner2.C?[] C2) opCSeqResult = await i.OpCSeqAsync(cseq1);
                        TestHelper.Assert(opCSeqResult.ReturnValue[0]!.S.Equals(s1));
                        TestHelper.Assert(opCSeqResult.C2[0]!.S.Equals(s1));

                        var cmap1 = new Dictionary<string, Inner.Inner2.C?> { ["a"] = c1 };
                        (Dictionary<string, Inner.Inner2.C?> ReturnValue,
                         Dictionary<string, Inner.Inner2.C?> C2) opCMapResult = await i.OpCMapAsync(cmap1);
                        TestHelper.Assert(opCMapResult.ReturnValue["a"]!.S.Equals(s1));
                        TestHelper.Assert(opCMapResult.C2["a"]!.S.Equals(s1));
                    }).Wait();
            }

            {
                var i = Inner.Inner2.IIPrx.Parse(helper.GetTestProxy("i3"), communicator);

                var s1 = new Inner.Inner2.S(0);
                var (s3, s2) = i.OpS(s1);
                TestHelper.Assert(s2.Equals(s1));
                TestHelper.Assert(s3.Equals(s1));

                var sseq1 = new Inner.Inner2.S[] { s1 };
                var (sseq3, sseq2) = i.OpSSeq(sseq1);
                TestHelper.Assert(sseq2[0].Equals(s1));
                TestHelper.Assert(sseq3[0].Equals(s1));

                var smap1 = new Dictionary<string, Inner.Inner2.S>();
                smap1["a"] = s1;
                var (smap3, smap2) = i.OpSMap(smap1);
                TestHelper.Assert(smap2["a"].Equals(s1));
                TestHelper.Assert(smap3["a"].Equals(s1));

                var c1 = new Inner.Inner2.C(s1);
                var (c3, c2) = i.OpC(c1);
                TestHelper.Assert(c2!.S.Equals(c1.S));
                TestHelper.Assert(c3!.S.Equals(c1.S));

                var cseq1 = new Inner.Inner2.C[] { c1 };
                var (cseq3, cseq2) = i.OpCSeq(cseq1);
                TestHelper.Assert(cseq2[0]!.S.Equals(s1));
                TestHelper.Assert(cseq3[0]!.S.Equals(s1));

                var cmap1 = new Dictionary<string, Inner.Inner2.C?> { ["a"] = c1 };
                (Dictionary<string, Inner.Inner2.C?> cmap3,
                 Dictionary<string, Inner.Inner2.C?> cmap2) = i.OpCMap(cmap1);
                TestHelper.Assert(cmap2["a"]!.S.Equals(s1));
                TestHelper.Assert(cmap3["a"]!.S.Equals(s1));
            }

            {
                var i = Inner.Inner2.IIPrx.Parse(helper.GetTestProxy("i3"), communicator);

                Task.Run(async () =>
                    {
                        var s1 = new Inner.Inner2.S(0);
                        (Inner.Inner2.S ReturnValue, Inner.Inner2.S s2) = await i.OpSAsync(s1);
                        TestHelper.Assert(s1.Equals(ReturnValue));
                        TestHelper.Assert(s1.Equals(s2));

                        var sseq1 = new Inner.Inner2.S[] { s1 };
                        (Inner.Inner2.S[] ReturnValue, Inner.Inner2.S[] s2) opSSeqResult = await i.OpSSeqAsync(sseq1);
                        TestHelper.Assert(opSSeqResult.ReturnValue[0].Equals(s1));
                        TestHelper.Assert(opSSeqResult.s2[0].Equals(s1));

                        var smap1 = new Dictionary<string, Inner.Inner2.S> { ["a"] = s1 };
                        var opSMapResult = await i.OpSMapAsync(smap1);
                        TestHelper.Assert(opSMapResult.R1["a"].Equals(s1));
                        TestHelper.Assert(opSMapResult.R2["a"].Equals(s1));

                        var c1 = new Inner.Inner2.C(s1);
                        var opCResult = await i.OpCAsync(c1);
                        TestHelper.Assert(c1.S.Equals(opCResult.R1!.S));
                        TestHelper.Assert(c1.S.Equals(opCResult.R2!.S));

                        Inner.Inner2.C[] cseq1 = new Inner.Inner2.C[] { c1 };
                        var opCSeqResult = await i.OpCSeqAsync(cseq1);
                        TestHelper.Assert(opCSeqResult.R1[0]!.S.Equals(s1));
                        TestHelper.Assert(opCSeqResult.R2[0]!.S.Equals(s1));

                        var cmap1 = new Dictionary<string, Inner.Inner2.C?> { ["a"] = c1 };
                        var opCMapResult = await i.OpCMapAsync(cmap1);
                        TestHelper.Assert(opCMapResult.R1["a"]!.S.Equals(s1));
                        TestHelper.Assert(opCMapResult.R2["a"]!.S.Equals(s1));
                    }).Wait();
            }

            {
                var i = Inner.Test.Inner2.IIPrx.Parse(helper.GetTestProxy("i4"), communicator);

                var s1 = new S(0);
                var (s3, s2) = i.OpS(s1);
                TestHelper.Assert(s2.Equals(s1));
                TestHelper.Assert(s3.Equals(s1));

                var sseq1 = new S[] { s1 };
                var (sseq3, sseq2) = i.OpSSeq(sseq1);
                TestHelper.Assert(sseq2[0].Equals(s1));
                TestHelper.Assert(sseq3[0].Equals(s1));

                var smap1 = new Dictionary<string, S>();
                smap1["a"] = s1;
                var (smap3, smap2) = i.OpSMap(smap1);
                TestHelper.Assert(smap2["a"].Equals(s1));
                TestHelper.Assert(smap3["a"].Equals(s1));

                var c1 = new C(s1);
                var (c3, c2) = i.OpC(c1);
                TestHelper.Assert(c2!.S.Equals(c1.S));
                TestHelper.Assert(c3!.S.Equals(c1.S));

                var cseq1 = new C[] { c1 };
                var (cseq3, cseq2) = i.OpCSeq(cseq1);
                TestHelper.Assert(cseq2[0]!.S.Equals(s1));
                TestHelper.Assert(cseq3[0]!.S.Equals(s1));

                var cmap1 = new Dictionary<string, C?> { ["a"] = c1 };
                var (cmap3, cmap2) = i.OpCMap(cmap1);
                TestHelper.Assert(cmap2["a"]!.S.Equals(s1));
                TestHelper.Assert(cmap3["a"]!.S.Equals(s1));
            }

            {
                var i = Inner.Test.Inner2.IIPrx.Parse(helper.GetTestProxy("i4"), communicator);

                Task.Run(async () =>
                    {
                        var s1 = new S(0);
                        (S ReturnValue, S s2) = await i.OpSAsync(s1);
                        TestHelper.Assert(s1.Equals(ReturnValue));
                        TestHelper.Assert(s1.Equals(s2));

                        var sseq1 = new S[] { s1 };
                        (S[] sseq2, S[] sseq3) opSSeqResult = await i.OpSSeqAsync(sseq1);
                        TestHelper.Assert(opSSeqResult.sseq2[0].Equals(s1));
                        TestHelper.Assert(opSSeqResult.sseq3[0].Equals(s1));

                        var smap1 = new Dictionary<string, S> { ["a"] = s1 };
                        (Dictionary<string, S> smap2, Dictionary<string, S> smap3) = await i.OpSMapAsync(smap1);
                        TestHelper.Assert(smap2["a"].Equals(s1));
                        TestHelper.Assert(smap3["a"].Equals(s1));

                        var c1 = new C(s1);
                        (C? c2, C? c3) = await i.OpCAsync(c1);
                        TestHelper.Assert(c1.S.Equals(c2!.S));
                        TestHelper.Assert(c1.S.Equals(c3!.S));

                        var cseq1 = new C[] { c1 };
                        (C?[] cseq2, C?[] cseq3) = await i.OpCSeqAsync(cseq1);
                        TestHelper.Assert(cseq2[0]!.S.Equals(s1));
                        TestHelper.Assert(cseq3[0]!.S.Equals(s1));

                        var cmap1 = new Dictionary<string, C?>
                        {
                            ["a"] = c1
                        };
                        (Dictionary<string, C?> cmap2, Dictionary<string, C?> cmap3) = await i.OpCMapAsync(cmap1);
                        TestHelper.Assert(cmap2["a"]!.S.Equals(s1));
                        TestHelper.Assert(cmap3["a"]!.S.Equals(s1));
                    }).Wait();
            }

            {
                IIPrx.Parse(helper.GetTestProxy("i1"), communicator).Shutdown();
            }

            output.WriteLine("ok");
            return Task.CompletedTask;
        }
    }
}
