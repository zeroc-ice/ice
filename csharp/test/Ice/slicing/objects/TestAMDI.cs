// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections.Generic;
using Test;

public sealed class TestI : TestIntfDisp_
{
    private static void test(bool b)
    {
        if(!b)
        {
            throw new Exception();
        }
    }

    public override void shutdownAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
        response();
    }

    public override void
    SBaseAsObjectAsync(Action<Ice.Object> response, Action<Exception> exception, Ice.Current current)
    {
        response(new SBase("SBase.sb"));
    }

    public override void SBaseAsSBaseAsync(Action<SBase> response, Action<Exception> exception, Ice.Current current)
    {
        response(new SBase("SBase.sb"));
    }

    public override void
    SBSKnownDerivedAsSBaseAsync(Action<SBase> response, Action<Exception> exception, Ice.Current current)
    {
        response(new SBSKnownDerived("SBSKnownDerived.sb", "SBSKnownDerived.sbskd"));
    }

    public override void
    SBSKnownDerivedAsSBSKnownDerivedAsync(Action<SBSKnownDerived> response, Action<Exception> exception,
                                          Ice.Current current)
    {
        response(new SBSKnownDerived("SBSKnownDerived.sb", "SBSKnownDerived.sbskd"));
    }

    public override void
    SBSUnknownDerivedAsSBaseAsync(Action<SBase> response, Action<Exception> exception, Ice.Current current)
    {
        response(new SBSUnknownDerived("SBSUnknownDerived.sb", "SBSUnknownDerived.sbsud"));
    }

    public override void
    SBSUnknownDerivedAsSBaseCompactAsync(Action<SBase> response, Action<Exception> exception, Ice.Current current)
    {
        response(new SBSUnknownDerived("SBSUnknownDerived.sb", "SBSUnknownDerived.sbsud"));
    }

    public override void SUnknownAsObjectAsync(Action<Ice.Object> response, Action<Exception> exception, Ice.Current current)
    {
        response(new SUnknown("SUnknown.su"));
    }

    public override void checkSUnknownAsync(Ice.Object obj, Action response, Action<Exception> exception, Ice.Current current)
    {
        if(current.encoding.Equals(Ice.Util.Encoding_1_0))
        {
            test(!(obj is SUnknown));
        }
        else
        {
            SUnknown su = obj as SUnknown;
            test(su.su.Equals("SUnknown.su"));
        }
        response();
    }

    public override void oneElementCycleAsync(Action<B> response, Action<Exception> exception, Ice.Current current)
    {
        B b = new B();
        b.sb = "B1.sb";
        b.pb = b;
        response(b);
    }

    public override void twoElementCycleAsync(Action<B> response, Action<Exception> exception, Ice.Current current)
    {
        B b1 = new B();
        b1.sb = "B1.sb";
        B b2 = new B();
        b2.sb = "B2.sb";
        b2.pb = b1;
        b1.pb = b2;
        response(b1);
    }

    public override void D1AsBAsync(Action<B> response, Action<Exception> exception, Ice.Current current)
    {
        D1 d1 = new D1();
        d1.sb = "D1.sb";
        d1.sd1 = "D1.sd1";
        D2 d2 = new D2();
        d2.pb = d1;
        d2.sb = "D2.sb";
        d2.sd2 = "D2.sd2";
        d2.pd2 = d1;
        d1.pb = d2;
        d1.pd1 = d2;
        response(d1);
    }

    public override void D1AsD1Async(Action<D1> response, Action<Exception> exception, Ice.Current current)
    {
        D1 d1 = new D1();
        d1.sb = "D1.sb";
        d1.sd1 = "D1.sd1";
        D2 d2 = new D2();
        d2.pb = d1;
        d2.sb = "D2.sb";
        d2.sd2 = "D2.sd2";
        d2.pd2 = d1;
        d1.pb = d2;
        d1.pd1 = d2;
        response(d1);
    }

    public override void D2AsBAsync(Action<B> response, Action<Exception> exception, Ice.Current current)
    {
        D2 d2 = new D2();
        d2.sb = "D2.sb";
        d2.sd2 = "D2.sd2";
        D1 d1 = new D1();
        d1.pb = d2;
        d1.sb = "D1.sb";
        d1.sd1 = "D1.sd1";
        d1.pd1 = d2;
        d2.pb = d1;
        d2.pd2 = d1;
        response(d2);
    }

    public override void
    paramTest1Async(Action<TestIntf_ParamTest1Result> response, Action<Exception> exception, Ice.Current current)
    {
        D1 d1 = new D1();
        d1.sb = "D1.sb";
        d1.sd1 = "D1.sd1";
        D2 d2 = new D2();
        d2.pb = d1;
        d2.sb = "D2.sb";
        d2.sd2 = "D2.sd2";
        d2.pd2 = d1;
        d1.pb = d2;
        d1.pd1 = d2;
        response(new TestIntf_ParamTest1Result(d1, d2));
    }

    public override void
    paramTest2Async(Action<TestIntf_ParamTest2Result> response, Action<Exception> exception, Ice.Current current)
    {
        D1 d1 = new D1();
        d1.sb = "D1.sb";
        d1.sd1 = "D1.sd1";
        D2 d2 = new D2();
        d2.pb = d1;
        d2.sb = "D2.sb";
        d2.sd2 = "D2.sd2";
        d2.pd2 = d1;
        d1.pb = d2;
        d1.pd1 = d2;
        response(new TestIntf_ParamTest2Result(d2, d1)); 
    }

    public override void
    paramTest3Async(Action<TestIntf_ParamTest3Result> response, Action<Exception> exception, Ice.Current current)
    {
        D2 d2 = new D2();
        d2.sb = "D2.sb (p1 1)";
        d2.pb = null;
        d2.sd2 = "D2.sd2 (p1 1)";

        D1 d1 = new D1();
        d1.sb = "D1.sb (p1 2)";
        d1.pb = null;
        d1.sd1 = "D1.sd2 (p1 2)";
        d1.pd1 = null;
        d2.pd2 = d1;

        D2 d4 = new D2();
        d4.sb = "D2.sb (p2 1)";
        d4.pb = null;
        d4.sd2 = "D2.sd2 (p2 1)";

        D1 d3 = new D1();
        d3.sb = "D1.sb (p2 2)";
        d3.pb = null;
        d3.sd1 = "D1.sd2 (p2 2)";
        d3.pd1 = null;
        d4.pd2 = d3;

        response(new TestIntf_ParamTest3Result(d3, d2, d4));
    }

    public override void
    paramTest4Async(Action<TestIntf_ParamTest4Result> response, Action<Exception> exception, Ice.Current current)
    {
        D4 d4 = new D4();
        d4.sb = "D4.sb (1)";
        d4.pb = null;
        d4.p1 = new B();
        d4.p1.sb = "B.sb (1)";
        d4.p2 = new B();
        d4.p2.sb = "B.sb (2)";
        response(new TestIntf_ParamTest4Result(d4.p2, d4));
    }

    public override void
    returnTest1Async(Action<TestIntf_ReturnTest1Result> response, Action<Exception> exception, Ice.Current current)
    {
        D1 d1 = new D1();
        d1.sb = "D1.sb";
        d1.sd1 = "D1.sd1";
        D2 d2 = new D2();
        d2.pb = d1;
        d2.sb = "D2.sb";
        d2.sd2 = "D2.sd2";
        d2.pd2 = d1;
        d1.pb = d2;
        d1.pd1 = d2;
        response(new TestIntf_ReturnTest1Result(d2, d2, d1));
    }

    public override void
    returnTest2Async(Action<TestIntf_ReturnTest2Result> response, Action<Exception> exception, Ice.Current current)
    {
        D1 d1 = new D1();
        d1.sb = "D1.sb";
        d1.sd1 = "D1.sd1";
        D2 d2 = new D2();
        d2.pb = d1;
        d2.sb = "D2.sb";
        d2.sd2 = "D2.sd2";
        d2.pd2 = d1;
        d1.pb = d2;
        d1.pd1 = d2;
        response(new TestIntf_ReturnTest2Result(d1, d1, d2));
    }

    public override void
    returnTest3Async(B p1, B p2, Action<B> response, Action<Exception> exception, Ice.Current current)
    {
        response(p1);
    }

    public override void
    sequenceTestAsync(SS1 p1, SS2 p2, Action<SS3> response, Action<Exception> exception, Ice.Current current)
    {
        response(new SS3(p1, p2));
    }

    public override void
    dictionaryTestAsync(Dictionary<int, B> bin, Action<TestIntf_DictionaryTestResult> response,
                        Action<Exception> exception, Ice.Current current)
    {
        var bout = new Dictionary<int, B>();
        int i;
        for(i = 0; i < 10; ++i)
        {
            B b = bin[i];
            var d2 = new D2();
            d2.sb = b.sb;
            d2.pb = b.pb;
            d2.sd2 = "D2";
            d2.pd2 = d2;
            bout[i * 10] = d2;
        }
        var r = new Dictionary<int, B>();
        for(i = 0; i < 10; ++i)
        {
            string s = "D1." + (i * 20).ToString();
            var d1 = new D1();
            d1.sb = s;
            d1.pb = (i == 0 ? null : r[(i - 1) * 20]);
            d1.sd1 = s;
            d1.pd1 = d1;
            r[i * 20] = d1;
        }
        response(new TestIntf_DictionaryTestResult(r, bout));
    }

    public override void
    exchangePBaseAsync(PBase pb, Action<PBase> response, Action<Exception> exception, Ice.Current current)
    {
        response(pb);
    }

    public override void
    PBSUnknownAsPreservedAsync(Action<Preserved> response, Action<Exception> exception, Ice.Current current)
    {
        PSUnknown r = new PSUnknown();
        r.pi = 5;
        r.ps = "preserved";
        r.psu = "unknown";
        r.graph = null;
        if(!current.encoding.Equals(Ice.Util.Encoding_1_0))
        {
            //
            // 1.0 encoding doesn't support unmarshaling unknown classes even if referenced
            // from unread slice.
            //
            r.cl = new MyClass(15);
        }
        response(r);
    }

    public override void
    checkPBSUnknownAsync(Preserved p, Action response, Action<Exception> exception, Ice.Current current)
    {
        if(current.encoding.Equals(Ice.Util.Encoding_1_0))
        {
            test(!(p is PSUnknown));
            test(p.pi == 5);
            test(p.ps.Equals("preserved"));
        }
        else
        {
            var pu = p as PSUnknown;
            test(pu.pi == 5);
            test(pu.ps.Equals("preserved"));
            test(pu.psu.Equals("unknown"));
            test(pu.graph == null);
            test(pu.cl != null && pu.cl.i == 15);
        }
        response();
    }

    public override void
    PBSUnknownAsPreservedWithGraphAsync(Action<Preserved> response, Action<Exception> exception, Ice.Current current)
    {
        var r = new PSUnknown();
        r.pi = 5;
        r.ps = "preserved";
        r.psu = "unknown";
        r.graph = new PNode();
        r.graph.next = new PNode();
        r.graph.next.next = new PNode();
        r.graph.next.next.next = r.graph;
        response(r);
    }

    public override void
    checkPBSUnknownWithGraphAsync(Preserved p, Action response, Action<Exception> exception, Ice.Current current)
    {
        if(current.encoding.Equals(Ice.Util.Encoding_1_0))
        {
            test(!(p is PSUnknown));
            test(p.pi == 5);
            test(p.ps.Equals("preserved"));
        }
        else
        {
            var pu = p as PSUnknown;
            test(pu.pi == 5);
            test(pu.ps.Equals("preserved"));
            test(pu.psu.Equals("unknown"));
            test(pu.graph != pu.graph.next);
            test(pu.graph.next != pu.graph.next.next);
            test(pu.graph.next.next.next == pu.graph);
        }
        response();
    }

    public override void
    PBSUnknown2AsPreservedWithGraphAsync(Action<Preserved> response, Action<Exception> exception, Ice.Current current)
    {
        var r = new PSUnknown2();
        r.pi = 5;
        r.ps = "preserved";
        r.pb = r;
        response(r);
    }

    public override void
    checkPBSUnknown2WithGraphAsync(Preserved p, Action response, Action<Exception> exception, Ice.Current current)
    {
        if(current.encoding.Equals(Ice.Util.Encoding_1_0))
        {
            test(!(p is PSUnknown2));
            test(p.pi == 5);
            test(p.ps.Equals("preserved"));
        }
        else
        {
            var pu = p as PSUnknown2;
            test(pu.pi == 5);
            test(pu.ps.Equals("preserved"));
            test(pu.pb == pu);
        }
        response();
    }

    public override void
    exchangePNodeAsync(PNode pn, Action<PNode> response, Action<Exception> exception, Ice.Current current)
    {
        response(pn);
    }

    public override void throwBaseAsBaseAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        var be = new BaseException();
        be.sbe = "sbe";
        be.pb = new B();
        be.pb.sb = "sb";
        be.pb.pb = be.pb;
        exception(be);
    }

    public override void throwDerivedAsBaseAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        DerivedException de = new DerivedException();
        de.sbe = "sbe";
        de.pb = new B();
        de.pb.sb = "sb1";
        de.pb.pb = de.pb;
        de.sde = "sde1";
        de.pd1 = new D1();
        de.pd1.sb = "sb2";
        de.pd1.pb = de.pd1;
        de.pd1.sd1 = "sd2";
        de.pd1.pd1 = de.pd1;
        exception(de);
    }

    public override
    void throwDerivedAsDerivedAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        var de = new DerivedException();
        de.sbe = "sbe";
        de.pb = new B();
        de.pb.sb = "sb1";
        de.pb.pb = de.pb;
        de.sde = "sde1";
        de.pd1 = new D1();
        de.pd1.sb = "sb2";
        de.pd1.pb = de.pd1;
        de.pd1.sd1 = "sd2";
        de.pd1.pd1 = de.pd1;
        exception(de);
    }

    public override void throwUnknownDerivedAsBaseAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        var d2 = new D2();
        d2.sb = "sb d2";
        d2.pb = d2;
        d2.sd2 = "sd2 d2";
        d2.pd2 = d2;

        var ude = new UnknownDerivedException();
        ude.sbe = "sbe";
        ude.pb = d2;
        ude.sude = "sude";
        ude.pd2 = d2;

        exception(ude);
    }

    public override void throwPreservedExceptionAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        var ue = new PSUnknownException();
        ue.p = new PSUnknown2();
        ue.p.pi = 5;
        ue.p.ps = "preserved";
        ue.p.pb = ue.p;

        exception(ue);
    }

    public override void
    useForwardAsync(Action<Forward> response, Action<Exception> exception, Ice.Current current)
    {
        var f = new Forward();
        f = new Forward();
        f.h = new Hidden();
        f.h.f = f;
        response(f);
    }
}
