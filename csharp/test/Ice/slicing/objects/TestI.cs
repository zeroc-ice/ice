//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Test;

public sealed class TestI : TestIntf
{
    private static void test(bool b)
    {
        if (!b)
        {
            throw new Exception();
        }
    }

    public void shutdown(Ice.Current current)
    {
        current.Adapter.Communicator.shutdown();
    }

    public Ice.Value SBaseAsObject(Ice.Current current)
    {
        SBase sb = new SBase();
        sb.sb = "SBase.sb";
        return sb;
    }

    public SBase SBaseAsSBase(Ice.Current current)
    {
        SBase sb = new SBase();
        sb.sb = "SBase.sb";
        return sb;
    }

    public SBase SBSKnownDerivedAsSBase(Ice.Current current)
    {
        SBSKnownDerived sbskd = new SBSKnownDerived();
        sbskd.sb = "SBSKnownDerived.sb";
        sbskd.sbskd = "SBSKnownDerived.sbskd";
        return sbskd;
    }

    public SBSKnownDerived SBSKnownDerivedAsSBSKnownDerived(Ice.Current current)
    {
        SBSKnownDerived sbskd = new SBSKnownDerived();
        sbskd.sb = "SBSKnownDerived.sb";
        sbskd.sbskd = "SBSKnownDerived.sbskd";
        return sbskd;
    }

    public SBase SBSUnknownDerivedAsSBase(Ice.Current current)
    {
        SBSUnknownDerived sbsud = new SBSUnknownDerived();
        sbsud.sb = "SBSUnknownDerived.sb";
        sbsud.sbsud = "SBSUnknownDerived.sbsud";
        return sbsud;
    }

    public SBase SBSUnknownDerivedAsSBaseCompact(Ice.Current current)
    {
        SBSUnknownDerived sbsud = new SBSUnknownDerived();
        sbsud.sb = "SBSUnknownDerived.sb";
        sbsud.sbsud = "SBSUnknownDerived.sbsud";
        return sbsud;
    }

    public Ice.Value SUnknownAsObject(Ice.Current current)
    {
        SUnknown su = new SUnknown();
        su.su = "SUnknown.su";
        su.cycle = su;
        return su;
    }

    public void checkSUnknown(Ice.Value obj, Ice.Current current)
    {
        if (current.Encoding.Equals(Ice.Util.Encoding_1_0))
        {
            test(!(obj is SUnknown));
        }
        else
        {
            test(obj is SUnknown);
            SUnknown su = (SUnknown)obj;
            test(su.su.Equals("SUnknown.su"));
        }
    }

    public B oneElementCycle(Ice.Current current)
    {
        B b = new B();
        b.sb = "B1.sb";
        b.pb = b;
        return b;
    }

    public B twoElementCycle(Ice.Current current)
    {
        B b1 = new B();
        b1.sb = "B1.sb";
        B b2 = new B();
        b2.sb = "B2.sb";
        b2.pb = b1;
        b1.pb = b2;
        return b1;
    }

    public B D1AsB(Ice.Current current)
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

    public D1 D1AsD1(Ice.Current current)
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

    public B D2AsB(Ice.Current current)
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

    public TestIntf.ParamTest1ReturnValue paramTest1(Ice.Current current)
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
        return new TestIntf.ParamTest1ReturnValue(d1, d2);
    }

    public TestIntf.ParamTest2ReturnValue paramTest2(Ice.Current current)
    {
        var r = paramTest1(current);
        return new TestIntf.ParamTest2ReturnValue(r.p2, r.p1);
    }

    public TestIntf.ParamTest3ReturnValue paramTest3(Ice.Current current)
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

        return new TestIntf.ParamTest3ReturnValue(d3, d2, d4);
    }

    public TestIntf.ParamTest4ReturnValue paramTest4(Ice.Current current)
    {
        D4 d4 = new D4();
        d4.sb = "D4.sb (1)";
        d4.pb = null;
        d4.p1 = new B();
        d4.p1.sb = "B.sb (1)";
        d4.p2 = new B();
        d4.p2.sb = "B.sb (2)";
        return new TestIntf.ParamTest4ReturnValue(d4.p2, d4);
    }

    public TestIntf.ReturnTest1ReturnValue returnTest1(Ice.Current current)
    {
        var r = paramTest1(current);
        return new TestIntf.ReturnTest1ReturnValue(r.p1, r.p1, r.p2);
    }

    public TestIntf.ReturnTest2ReturnValue returnTest2(Ice.Current current)
    {
        var r = paramTest1(current);
        return new TestIntf.ReturnTest2ReturnValue(r.p2, r.p2, r.p1);
    }

    public B returnTest3(B p1, B p2, Ice.Current current)
    {
        return p1;
    }

    public SS3 sequenceTest(SS1 p1, SS2 p2, Ice.Current current)
    {
        SS3 ss = new SS3();
        ss.c1 = p1;
        ss.c2 = p2;
        return ss;
    }

    public TestIntf.DictionaryTestReturnValue dictionaryTest(Dictionary<int, B> bin, Ice.Current current)
    {
        var bout = new Dictionary<int, B>();
        int i;
        for (i = 0; i < 10; ++i)
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
        for (i = 0; i < 10; ++i)
        {
            string s = "D1." + (i * 20).ToString();
            D1 d1 = new D1();
            d1.sb = s;
            d1.pb = i == 0 ? null : r[(i - 1) * 20];
            d1.sd1 = s;
            d1.pd1 = d1;
            r[i * 20] = d1;
        }
        return new TestIntf.DictionaryTestReturnValue(r, bout);
    }

    public PBase exchangePBase(PBase pb, Ice.Current current)
    {
        return pb;
    }

    public Preserved PBSUnknownAsPreserved(Ice.Current current)
    {
        PSUnknown r = new PSUnknown();
        r.pi = 5;
        r.ps = "preserved";
        r.psu = "unknown";
        r.graph = null;
        if (!current.Encoding.Equals(Ice.Util.Encoding_1_0))
        {
            //
            // 1.0 encoding doesn't support unmarshaling unknown classes even if referenced
            // from unread slice.
            //
            r.cl = new MyClass(15);
        }
        return r;
    }

    public void checkPBSUnknown(Preserved p, Ice.Current current)
    {
        if (current.Encoding.Equals(Ice.Util.Encoding_1_0))
        {
            test(!(p is PSUnknown));
            test(p.pi == 5);
            test(p.ps.Equals("preserved"));
        }
        else
        {
            test(p is PSUnknown);
            PSUnknown pu = (PSUnknown)p;
            test(pu.pi == 5);
            test(pu.ps.Equals("preserved"));
            test(pu.psu.Equals("unknown"));
            test(pu.graph == null);
            test(pu.cl != null && pu.cl.i == 15);
        }
    }

    public Task<Preserved>
    PBSUnknownAsPreservedWithGraphAsync(Ice.Current current)
    {
        var r = new PSUnknown();
        r.pi = 5;
        r.ps = "preserved";
        r.psu = "unknown";
        r.graph = new PNode();
        r.graph.next = new PNode();
        r.graph.next.next = new PNode();
        r.graph.next.next.next = r.graph;
        return Task.FromResult<Preserved>(r);
    }

    public void checkPBSUnknownWithGraph(Preserved p, Ice.Current current)
    {
        if (current.Encoding.Equals(Ice.Util.Encoding_1_0))
        {
            test(!(p is PSUnknown));
            test(p.pi == 5);
            test(p.ps.Equals("preserved"));
        }
        else
        {
            test(p is PSUnknown);
            PSUnknown pu = (PSUnknown)p;
            test(pu.pi == 5);
            test(pu.ps.Equals("preserved"));
            test(pu.psu.Equals("unknown"));
            test(pu.graph != pu.graph.next);
            test(pu.graph.next != pu.graph.next.next);
            test(pu.graph.next.next.next == pu.graph);
        }
    }

    public Task<Preserved>
    PBSUnknown2AsPreservedWithGraphAsync(Ice.Current current)
    {
        var r = new PSUnknown2();
        r.pi = 5;
        r.ps = "preserved";
        r.pb = r;
        return Task.FromResult<Preserved>(r);
    }

    public void checkPBSUnknown2WithGraph(Preserved p, Ice.Current current)
    {
        if (current.Encoding.Equals(Ice.Util.Encoding_1_0))
        {
            test(!(p is PSUnknown2));
            test(p.pi == 5);
            test(p.ps.Equals("preserved"));
        }
        else
        {
            test(p is PSUnknown2);
            PSUnknown2 pu = (PSUnknown2)p;
            test(pu.pi == 5);
            test(pu.ps.Equals("preserved"));
            test(pu.pb == pu);
        }
    }

    public PNode exchangePNode(PNode pn, Ice.Current current)
    {
        return pn;
    }

    public void throwBaseAsBase(Ice.Current current)
    {
        BaseException be = new BaseException();
        be.sbe = "sbe";
        be.pb = new B();
        be.pb.sb = "sb";
        be.pb.pb = be.pb;
        throw be;
    }

    public void throwDerivedAsBase(Ice.Current current)
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

    public void throwDerivedAsDerived(Ice.Current current)
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

    public void throwUnknownDerivedAsBase(Ice.Current current)
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

    public Task
    throwPreservedExceptionAsync(Ice.Current current)
    {
        var ue = new PSUnknownException();
        ue.p = new PSUnknown2();
        ue.p.pi = 5;
        ue.p.ps = "preserved";
        ue.p.pb = ue.p;
        throw ue;
    }

    public Forward useForward(Ice.Current current)
    {
        var f = new Forward();
        f.h = new Hidden();
        f.h.f = f;
        return f;
    }
}
