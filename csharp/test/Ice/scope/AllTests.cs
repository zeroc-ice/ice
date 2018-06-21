// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Test;

public class AllTests : Test.AllTests
{
    public static void allTests(TestHelper helper)
    {
        Ice.Communicator communicator = helper.communicator();
        {
            Ice.ObjectPrx obj = communicator.stringToProxy("i1:" + helper.getTestEndpoint());
            IPrx i = IPrxHelper.checkedCast(obj);

            S s1 = new S(0);
            S s2;
            S s3 = i.opS(s1, out s2);
            test(s2.Equals(s1));
            test(s3.Equals(s1));

            S[] sseq1 = new S[] { s1 };
            S[] sseq2;
            S[] sseq3 = i.opSSeq(sseq1, out sseq2);
            test(sseq2[0].Equals(s1));
            test(sseq3[0].Equals(s1));

            Dictionary<String, S> smap1 = new Dictionary<String, S>();
            smap1["a"] = s1;
            Dictionary<String, S> smap2;
            Dictionary<String, S> smap3 = i.opSMap(smap1, out smap2);
            test(smap2["a"].Equals(s1));
            test(smap3["a"].Equals(s1));

            C c1 = new C(s1);
            C c2;
            C c3 = i.opC(c1, out c2);
            test(c2.s.Equals(c1.s));
            test(c3.s.Equals(c1.s));

            C[] cseq1 = new C[] { c1 };
            C[] cseq2;
            C[] cseq3 = i.opCSeq(cseq1, out cseq2);
            test(cseq2[0].s.Equals(s1));
            test(cseq3[0].s.Equals(s1));

            Dictionary<String, C> cmap1 = new Dictionary<String, C>();
            cmap1["a"] = c1;
            Dictionary<String, C> cmap2;
            Dictionary<String, C> cmap3 = i.opCMap(cmap1, out cmap2);
            test(cmap2["a"].s.Equals(s1));
            test(cmap3["a"].s.Equals(s1));
        }

        {
            Ice.ObjectPrx obj = communicator.stringToProxy("i1:" + helper.getTestEndpoint());
            IPrx i = IPrxHelper.checkedCast(obj);

            Task.Run(async () =>
                {
                    S s1 = new S(0);
                    I_OpSResult opSResult = await i.opSAsync(s1);
                    test(s1.Equals(opSResult.returnValue));
                    test(s1.Equals(opSResult.s2));

                    S[] sseq1 = new S[] { s1 };
                    I_OpSSeqResult opSSeqResult = await i.opSSeqAsync(sseq1);
                    test(opSSeqResult.returnValue[0].Equals(s1));
                    test(opSSeqResult.s2[0].Equals(s1));

                    Dictionary<String, S> smap1 = new Dictionary<String, S>();
                    smap1["a"] = s1;
                    I_OpSMapResult opSMapResult = await i.opSMapAsync(smap1);
                    test(opSMapResult.returnValue["a"].Equals(s1));
                    test(opSMapResult.s2["a"].Equals(s1));

                    C c1 = new C(s1);
                    I_OpCResult opCResult = await i.opCAsync(c1);
                    test(c1.s.Equals(opCResult.returnValue.s));
                    test(c1.s.Equals(opCResult.c2.s));

                    C[] cseq1 = new C[] { c1 };
                    I_OpCSeqResult opCSeqResult = await i.opCSeqAsync(cseq1);
                    test(opCSeqResult.returnValue[0].s.Equals(s1));
                    test(opCSeqResult.c2[0].s.Equals(s1));

                    Dictionary<String, C> cmap1 = new Dictionary<String, C>();
                    cmap1["a"] = c1;
                    I_OpCMapResult opCMapResult = await i.opCMapAsync(cmap1);
                    test(opCMapResult.returnValue["a"].s.Equals(s1));
                    test(opCMapResult.c2["a"].s.Equals(s1));
                }).Wait();
        }

        {
            Ice.ObjectPrx obj = communicator.stringToProxy("i2:" + helper.getTestEndpoint());
            Test.Inner.IPrx i = Test.Inner.IPrxHelper.checkedCast(obj);

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
            Ice.ObjectPrx obj = communicator.stringToProxy("i2:" + helper.getTestEndpoint());
            Test.Inner.IPrx i = Test.Inner.IPrxHelper.checkedCast(obj);

            Task.Run(async () =>
                {
                    Test.Inner.Inner2.S s1 = new Test.Inner.Inner2.S(0);
                    Test.Inner.I_OpSResult opSResult = await i.opSAsync(s1);
                    test(s1.Equals(opSResult.returnValue));
                    test(s1.Equals(opSResult.s2));

                    Test.Inner.Inner2.S[] sseq1 = new Test.Inner.Inner2.S[] { s1 };
                    Test.Inner.I_OpSSeqResult opSSeqResult = await i.opSSeqAsync(sseq1);
                    test(opSSeqResult.returnValue[0].Equals(s1));
                    test(opSSeqResult.s2[0].Equals(s1));

                    Dictionary<String, Test.Inner.Inner2.S> smap1 = new Dictionary<String, Test.Inner.Inner2.S>();
                    smap1["a"] = s1;
                    Test.Inner.I_OpSMapResult opSMapResult = await i.opSMapAsync(smap1);
                    test(opSMapResult.returnValue["a"].Equals(s1));
                    test(opSMapResult.s2["a"].Equals(s1));

                    Test.Inner.Inner2.C c1 = new Test.Inner.Inner2.C(s1);
                    Test.Inner.I_OpCResult opCResult = await i.opCAsync(c1);
                    test(c1.s.Equals(opCResult.returnValue.s));
                    test(c1.s.Equals(opCResult.c2.s));

                    Test.Inner.Inner2.C[] cseq1 = new Test.Inner.Inner2.C[] { c1 };
                    Test.Inner.I_OpCSeqResult opCSeqResult = await i.opCSeqAsync(cseq1);
                    test(opCSeqResult.returnValue[0].s.Equals(s1));
                    test(opCSeqResult.c2[0].s.Equals(s1));

                    Dictionary<String, Test.Inner.Inner2.C> cmap1 = new Dictionary<String, Test.Inner.Inner2.C>();
                    cmap1["a"] = c1;
                    Test.Inner.I_OpCMapResult opCMapResult = await i.opCMapAsync(cmap1);
                    test(opCMapResult.returnValue["a"].s.Equals(s1));
                    test(opCMapResult.c2["a"].s.Equals(s1));
                }).Wait();
        }

        {
            Ice.ObjectPrx obj = communicator.stringToProxy("i3:" + helper.getTestEndpoint());
            Test.Inner.Inner2.IPrx i = Test.Inner.Inner2.IPrxHelper.checkedCast(obj);

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
            Ice.ObjectPrx obj = communicator.stringToProxy("i3:" + helper.getTestEndpoint());
            Test.Inner.Inner2.IPrx i = Test.Inner.Inner2.IPrxHelper.checkedCast(obj);

            Task.Run(async () =>
                {
                    Test.Inner.Inner2.S s1 = new Test.Inner.Inner2.S(0);
                    Test.Inner.Inner2.I_OpSResult opSResult = await i.opSAsync(s1);
                    test(s1.Equals(opSResult.returnValue));
                    test(s1.Equals(opSResult.s2));

                    Test.Inner.Inner2.S[] sseq1 = new Test.Inner.Inner2.S[] { s1 };
                    Test.Inner.Inner2.I_OpSSeqResult opSSeqResult = await i.opSSeqAsync(sseq1);
                    test(opSSeqResult.returnValue[0].Equals(s1));
                    test(opSSeqResult.s2[0].Equals(s1));

                    Dictionary<String, Test.Inner.Inner2.S> smap1 = new Dictionary<String, Test.Inner.Inner2.S>();
                    smap1["a"] = s1;
                    Test.Inner.Inner2.I_OpSMapResult opSMapResult = await i.opSMapAsync(smap1);
                    test(opSMapResult.returnValue["a"].Equals(s1));
                    test(opSMapResult.s2["a"].Equals(s1));

                    Test.Inner.Inner2.C c1 = new Test.Inner.Inner2.C(s1);
                    Test.Inner.Inner2.I_OpCResult opCResult = await i.opCAsync(c1);
                    test(c1.s.Equals(opCResult.returnValue.s));
                    test(c1.s.Equals(opCResult.c2.s));

                    Test.Inner.Inner2.C[] cseq1 = new Test.Inner.Inner2.C[] { c1 };
                    Test.Inner.Inner2.I_OpCSeqResult opCSeqResult = await i.opCSeqAsync(cseq1);
                    test(opCSeqResult.returnValue[0].s.Equals(s1));
                    test(opCSeqResult.c2[0].s.Equals(s1));

                    Dictionary<String, Test.Inner.Inner2.C> cmap1 = new Dictionary<String, Test.Inner.Inner2.C>();
                    cmap1["a"] = c1;
                    Test.Inner.Inner2.I_OpCMapResult opCMapResult = await i.opCMapAsync(cmap1);
                    test(opCMapResult.returnValue["a"].s.Equals(s1));
                    test(opCMapResult.c2["a"].s.Equals(s1));
                }).Wait();
        }

        {
            Ice.ObjectPrx obj = communicator.stringToProxy("i1:" + helper.getTestEndpoint());
            IPrx i = IPrxHelper.checkedCast(obj);
            i.shutdown();
        }
    }
}
