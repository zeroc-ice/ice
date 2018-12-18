// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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
                    var obj = communicator.stringToProxy("i1:" + helper.getTestEndpoint());
                    var i = Test.IPrxHelper.checkedCast(obj);

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
                }

                {
                    var obj = communicator.stringToProxy("i1:" + helper.getTestEndpoint());
                    var i = Test.IPrxHelper.checkedCast(obj);

                    Task.Run(async() =>
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
                        }).Wait();
                }

                {
                    var obj = communicator.stringToProxy("i2:" + helper.getTestEndpoint());
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
                    var obj = communicator.stringToProxy("i2:" + helper.getTestEndpoint());
                    Test.Inner.IPrx i = Test.Inner.IPrxHelper.checkedCast(obj);

                    Task.Run(async() =>
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
                    var obj = communicator.stringToProxy("i3:" + helper.getTestEndpoint());
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
                    var obj = communicator.stringToProxy("i3:" + helper.getTestEndpoint());
                    Test.Inner.Inner2.IPrx i = Test.Inner.Inner2.IPrxHelper.checkedCast(obj);

                    Task.Run(async() =>
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
                    var obj = communicator.stringToProxy("i4:" + helper.getTestEndpoint());
                    Inner.Test.Inner2.IPrx i = Inner.Test.Inner2.IPrxHelper.checkedCast(obj);

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
                    var obj = communicator.stringToProxy("i4:" + helper.getTestEndpoint());
                    Inner.Test.Inner2.IPrx i = Inner.Test.Inner2.IPrxHelper.checkedCast(obj);

                    Task.Run(async() =>
                        {
                            Test.S s1 = new Test.S(0);
                            Inner.Test.Inner2.I_OpSResult opSResult = await i.opSAsync(s1);
                            test(s1.Equals(opSResult.returnValue));
                            test(s1.Equals(opSResult.s2));

                            Test.S[] sseq1 = new Test.S[] { s1 };
                            Inner.Test.Inner2.I_OpSSeqResult opSSeqResult = await i.opSSeqAsync(sseq1);
                            test(opSSeqResult.returnValue[0].Equals(s1));
                            test(opSSeqResult.s2[0].Equals(s1));

                            Dictionary<String, Test.S> smap1 = new Dictionary<String, Test.S>();
                            smap1["a"] = s1;
                            Inner.Test.Inner2.I_OpSMapResult opSMapResult = await i.opSMapAsync(smap1);
                            test(opSMapResult.returnValue["a"].Equals(s1));
                            test(opSMapResult.s2["a"].Equals(s1));

                            Test.C c1 = new Test.C(s1);
                            Inner.Test.Inner2.I_OpCResult opCResult = await i.opCAsync(c1);
                            test(c1.s.Equals(opCResult.returnValue.s));
                            test(c1.s.Equals(opCResult.c2.s));

                            Test.C[] cseq1 = new Test.C[] { c1 };
                            Inner.Test.Inner2.I_OpCSeqResult opCSeqResult = await i.opCSeqAsync(cseq1);
                            test(opCSeqResult.returnValue[0].s.Equals(s1));
                            test(opCSeqResult.c2[0].s.Equals(s1));

                            Dictionary<String, Test.C> cmap1 = new Dictionary<String, Test.C>();
                            cmap1["a"] = c1;
                            Inner.Test.Inner2.I_OpCMapResult opCMapResult = await i.opCMapAsync(cmap1);
                            test(opCMapResult.returnValue["a"].s.Equals(s1));
                            test(opCMapResult.c2["a"].s.Equals(s1));
                        }).Wait();
                }

                {
                    var obj = communicator.stringToProxy("i1:" + helper.getTestEndpoint());
                    var i = Test.IPrxHelper.checkedCast(obj);
                    i.shutdown();
                }
            }
        }
    }
}
