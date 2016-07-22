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
            throw new System.Exception();
        }
    }

    public override void shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    public override Ice.Object SBaseAsObject(Ice.Current current)
    {
        SBase sb = new SBase();
        sb.sb = "SBase.sb";
        return sb;
    }

    public override SBase SBaseAsSBase(Ice.Current current)
    {
        SBase sb = new SBase();
        sb.sb = "SBase.sb";
        return sb;
    }

    public override SBase SBSKnownDerivedAsSBase(Ice.Current current)
    {
        SBSKnownDerived sbskd = new SBSKnownDerived();
        sbskd.sb = "SBSKnownDerived.sb";
        sbskd.sbskd = "SBSKnownDerived.sbskd";
        return sbskd;
    }

    public override SBSKnownDerived SBSKnownDerivedAsSBSKnownDerived(Ice.Current current)
    {
        SBSKnownDerived sbskd = new SBSKnownDerived();
        sbskd.sb = "SBSKnownDerived.sb";
        sbskd.sbskd = "SBSKnownDerived.sbskd";
        return sbskd;
    }

    public override SBase SBSUnknownDerivedAsSBase(Ice.Current current)
    {
        SBSUnknownDerived sbsud = new SBSUnknownDerived();
        sbsud.sb = "SBSUnknownDerived.sb";
        sbsud.sbsud = "SBSUnknownDerived.sbsud";
        return sbsud;
    }

    public override SBase SBSUnknownDerivedAsSBaseCompact(Ice.Current current)
    {
        SBSUnknownDerived sbsud = new SBSUnknownDerived();
        sbsud.sb = "SBSUnknownDerived.sb";
        sbsud.sbsud = "SBSUnknownDerived.sbsud";
        return sbsud;
    }

    public override Ice.Object SUnknownAsObject(Ice.Current current)
    {
        SUnknown su = new SUnknown();
        su.su = "SUnknown.su";
        return su;
    }

    public override void checkSUnknown(Ice.Object obj, Ice.Current current)
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
    }

    public override B oneElementCycle(Ice.Current current)
    {
        B b = new B();
        b.sb = "B1.sb";
        b.pb = b;
        return b;
    }

    public override B twoElementCycle(Ice.Current current)
    {
        B b1 = new B();
        b1.sb = "B1.sb";
        B b2 = new B();
        b2.sb = "B2.sb";
        b2.pb = b1;
        b1.pb = b2;
        return b1;
    }

    public override B D1AsB(Ice.Current current)
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
        return d1;
    }

    public override D1 D1AsD1(Ice.Current current)
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
        return d1;
    }

    public override B D2AsB(Ice.Current current)
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
        return d2;
    }

    public override void paramTest1(out B p1, out B p2, Ice.Current current)
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
        p1 = d1;
        p2 = d2;
    }

    public override void paramTest2(out B p1, out B p2, Ice.Current current)
    {
        paramTest1(out p2, out p1, current);
    }

    public override B paramTest3(out B p1, out B p2, Ice.Current current)
    {
        D2 d2 = new D2();
        d2.sb = "D2.sb (p1 1)";
        d2.pb = null;
        d2.sd2 = "D2.sd2 (p1 1)";
        p1 = d2;

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
        p2 = d4;

        D1 d3 = new D1();
        d3.sb = "D1.sb (p2 2)";
        d3.pb = null;
        d3.sd1 = "D1.sd2 (p2 2)";
        d3.pd1 = null;
        d4.pd2 = d3;

        return d3;
    }

    public override B paramTest4(out B p1, Ice.Current current)
    {
        D4 d4 = new D4();
        d4.sb = "D4.sb (1)";
        d4.pb = null;
        d4.p1 = new B();
        d4.p1.sb = "B.sb (1)";
        d4.p2 = new B();
        d4.p2.sb = "B.sb (2)";
        p1 = d4;
        return d4.p2;
    }

    public override B returnTest1(out B p1, out B p2, Ice.Current current)
    {
        paramTest1(out p1, out p2, current);
        return p1;
    }

    public override B returnTest2(out B p1, out B p2, Ice.Current current)
    {
        paramTest1(out p2, out p1, current);
        return p1;
    }

    public override B returnTest3(B p1, B p2, Ice.Current current)
    {
        return p1;
    }

    public override SS3 sequenceTest(SS1 p1, SS2 p2, Ice.Current current)
    {
        SS3 ss = new SS3();
        ss.c1 = p1;
        ss.c2 = p2;
        return ss;
    }

    public override Dictionary<int, B> dictionaryTest(Dictionary<int, B> bin, out Dictionary<int, B> bout,
                                                      Ice.Current current)
    {
        bout = new Dictionary<int, B>();
        int i;
        for(i = 0; i < 10; ++i)
        {
            B b = bin[i];
            D2 d2 = new D2();
            d2.sb = b.sb;
            d2.pb = b.pb;
            d2.sd2 = "D2";
            d2.pd2 = d2;
            bout[i * 10] = d2;
        }
        Dictionary<int, B> r = new Dictionary<int, B>();
        for(i = 0; i < 10; ++i)
        {
            string s = "D1." + (i * 20).ToString();
            D1 d1 = new D1();
            d1.sb = s;
            d1.pb = i == 0 ? null : r[(i - 1) * 20];
            d1.sd1 = s;
            d1.pd1 = d1;
            r[i * 20] = d1;
        }
        return r;
    }

    public override PBase exchangePBase(PBase pb, Ice.Current current)
    {
        return pb;
    }

    public override Preserved PBSUnknownAsPreserved(Ice.Current current)
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
        return r;
    }

    public override void checkPBSUnknown(Preserved p, Ice.Current current)
    {
        if(current.encoding.Equals(Ice.Util.Encoding_1_0))
        {
            test(!(p is PSUnknown));
            test(p.pi == 5);
            test(p.ps.Equals("preserved"));
        }
        else
        {
            PSUnknown pu = p as PSUnknown;
            test(pu.pi == 5);
            test(pu.ps.Equals("preserved"));
            test(pu.psu.Equals("unknown"));
            test(pu.graph == null);
            test(pu.cl != null && pu.cl.i == 15);
        }
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

    public override void checkPBSUnknownWithGraph(Preserved p, Ice.Current current)
    {
        if(current.encoding.Equals(Ice.Util.Encoding_1_0))
        {
            test(!(p is PSUnknown));
            test(p.pi == 5);
            test(p.ps.Equals("preserved"));
        }
        else
        {
            PSUnknown pu = p as PSUnknown;
            test(pu.pi == 5);
            test(pu.ps.Equals("preserved"));
            test(pu.psu.Equals("unknown"));
            test(pu.graph != pu.graph.next);
            test(pu.graph.next != pu.graph.next.next);
            test(pu.graph.next.next.next == pu.graph);
        }
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

    public override void checkPBSUnknown2WithGraph(Preserved p, Ice.Current current)
    {
        if(current.encoding.Equals(Ice.Util.Encoding_1_0))
        {
            test(!(p is PSUnknown2));
            test(p.pi == 5);
            test(p.ps.Equals("preserved"));
        }
        else
        {
            PSUnknown2 pu = p as PSUnknown2;
            test(pu.pi == 5);
            test(pu.ps.Equals("preserved"));
            test(pu.pb == pu);
        }
    }

    public override PNode exchangePNode(PNode pn, Ice.Current current)
    {
        return pn;
    }

    public override void throwBaseAsBase(Ice.Current current)
    {
        BaseException be = new BaseException();
        be.sbe = "sbe";
        be.pb = new B();
        be.pb.sb = "sb";
        be.pb.pb = be.pb;
        throw be;
    }

    public override void throwDerivedAsBase(Ice.Current current)
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
        throw de;
    }

    public override void throwDerivedAsDerived(Ice.Current current)
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
        throw de;
    }

    public override void throwUnknownDerivedAsBase(Ice.Current current)
    {
        D2 d2 = new D2();
        d2.sb = "sb d2";
        d2.pb = d2;
        d2.sd2 = "sd2 d2";
        d2.pd2 = d2;

        UnknownDerivedException ude = new UnknownDerivedException();
        ude.sbe = "sbe";
        ude.pb = d2;
        ude.sude = "sude";
        ude.pd2 = d2;
        throw ude;
    }

    public override void
    throwPreservedExceptionAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        var ue = new PSUnknownException();
        ue.p = new PSUnknown2();
        ue.p.pi = 5;
        ue.p.ps = "preserved";
        ue.p.pb = ue.p;
        exception(ue);
    }

    public override void useForward(out Forward f, Ice.Current current)
    {
        f = new Forward();
        f.h = new Hidden();
        f.h.f = f;
    }
}
