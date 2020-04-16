//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Test;

public sealed class TestIntf : ITestIntf
{
    public ValueTask shutdownAsync(Ice.Current current)
    {
        current.Adapter.Communicator.Shutdown();
        return new ValueTask(Task.CompletedTask);
    }

    public ValueTask<Ice.AnyClass?>
    SBaseAsObjectAsync(Ice.Current current) => FromResult<Ice.AnyClass?>(new SBase("SBase.sb"));

    public ValueTask<SBase?> SBaseAsSBaseAsync(Ice.Current current) => FromResult<SBase?>(new SBase("SBase.sb"));

    public ValueTask<SBase?>
    SBSKnownDerivedAsSBaseAsync(Ice.Current current) =>
        FromResult<SBase?>(new SBSKnownDerived("SBSKnownDerived.sb", "SBSKnownDerived.sbskd"));

    public ValueTask<SBSKnownDerived?>
    SBSKnownDerivedAsSBSKnownDerivedAsync(Ice.Current current) =>
        FromResult<SBSKnownDerived?>(new SBSKnownDerived("SBSKnownDerived.sb", "SBSKnownDerived.sbskd"));

    public ValueTask<SBase?>
    SBSUnknownDerivedAsSBaseAsync(Ice.Current current) =>
        FromResult<SBase?>(new SBSUnknownDerived("SBSUnknownDerived.sb", "SBSUnknownDerived.sbsud"));

    public ValueTask<SBase?>
    SBSUnknownDerivedAsSBaseCompactAsync(Ice.Current current) =>
        FromResult<SBase?>(new SBSUnknownDerived("SBSUnknownDerived.sb", "SBSUnknownDerived.sbsud"));

    public ValueTask<Ice.AnyClass?> SUnknownAsObjectAsync(Ice.Current current)
    {
        var su = new SUnknown("SUnknown.su", null);
        su.cycle = su;
        return FromResult<Ice.AnyClass?>(su);
    }

    public ValueTask checkSUnknownAsync(Ice.AnyClass? obj, Ice.Current current)
    {
        TestHelper.Assert(obj != null);
        var su = (SUnknown)obj;
        TestHelper.Assert(su.su.Equals("SUnknown.su"));
        return new ValueTask(Task.CompletedTask);
    }

    public ValueTask<B?> oneElementCycleAsync(Ice.Current current)
    {
        var b = new B();
        b.sb = "B1.sb";
        b.pb = b;
        return FromResult<B?>(b);
    }

    public ValueTask<B?> twoElementCycleAsync(Ice.Current current)
    {
        var b1 = new B();
        b1.sb = "B1.sb";
        var b2 = new B();
        b2.sb = "B2.sb";
        b2.pb = b1;
        b1.pb = b2;
        return FromResult<B?>(b1);
    }

    public ValueTask<B?> D1AsBAsync(Ice.Current current)
    {
        var d1 = new D1();
        d1.sb = "D1.sb";
        d1.sd1 = "D1.sd1";
        var d2 = new D2();
        d2.pb = d1;
        d2.sb = "D2.sb";
        d2.sd2 = "D2.sd2";
        d2.pd2 = d1;
        d1.pb = d2;
        d1.pd1 = d2;
        return FromResult<B?>(d1);
    }

    public ValueTask<D1?> D1AsD1Async(Ice.Current current)
    {
        var d1 = new D1();
        d1.sb = "D1.sb";
        d1.sd1 = "D1.sd1";
        var d2 = new D2();
        d2.pb = d1;
        d2.sb = "D2.sb";
        d2.sd2 = "D2.sd2";
        d2.pd2 = d1;
        d1.pb = d2;
        d1.pd1 = d2;
        return FromResult<D1?>(d1);
    }

    public ValueTask<B?> D2AsBAsync(Ice.Current current)
    {
        var d2 = new D2();
        d2.sb = "D2.sb";
        d2.sd2 = "D2.sd2";
        var d1 = new D1();
        d1.pb = d2;
        d1.sb = "D1.sb";
        d1.sd1 = "D1.sd1";
        d1.pd1 = d2;
        d2.pb = d1;
        d2.pd2 = d1;
        return FromResult<B?>(d2);
    }

    public ValueTask<(B?, B?)>
    paramTest1Async(Ice.Current current)
    {
        var d1 = new D1();
        d1.sb = "D1.sb";
        d1.sd1 = "D1.sd1";
        var d2 = new D2();
        d2.pb = d1;
        d2.sb = "D2.sb";
        d2.sd2 = "D2.sd2";
        d2.pd2 = d1;
        d1.pb = d2;
        d1.pd1 = d2;
        return FromResult(((B?)d1, (B?)d2));
    }

    public ValueTask<(B?, B?)>
    paramTest2Async(Ice.Current current)
    {
        var d1 = new D1();
        d1.sb = "D1.sb";
        d1.sd1 = "D1.sd1";
        var d2 = new D2();
        d2.pb = d1;
        d2.sb = "D2.sb";
        d2.sd2 = "D2.sd2";
        d2.pd2 = d1;
        d1.pb = d2;
        d1.pd1 = d2;
        return FromResult(((B?)d2, (B?)d1));
    }

    public ValueTask<(B?, B?, B?)>
    paramTest3Async(Ice.Current current)
    {
        var d2 = new D2();
        d2.sb = "D2.sb (p1 1)";
        d2.pb = null;
        d2.sd2 = "D2.sd2 (p1 1)";

        var d1 = new D1();
        d1.sb = "D1.sb (p1 2)";
        d1.pb = null;
        d1.sd1 = "D1.sd2 (p1 2)";
        d1.pd1 = null;
        d2.pd2 = d1;

        var d4 = new D2();
        d4.sb = "D2.sb (p2 1)";
        d4.pb = null;
        d4.sd2 = "D2.sd2 (p2 1)";

        var d3 = new D1();
        d3.sb = "D1.sb (p2 2)";
        d3.pb = null;
        d3.sd1 = "D1.sd2 (p2 2)";
        d3.pd1 = null;
        d4.pd2 = d3;

        return FromResult(((B?)d3, (B?)d2, (B?)d4));
    }

    public ValueTask<(B?, B?)>
    paramTest4Async(Ice.Current current)
    {
        var d4 = new D4();
        d4.sb = "D4.sb (1)";
        d4.pb = null;
        d4.p1 = new B();
        d4.p1.sb = "B.sb (1)";
        d4.p2 = new B();
        d4.p2.sb = "B.sb (2)";
        return FromResult(((B?)d4.p2, (B?)d4));
    }

    public ValueTask<(B?, B?, B?)>
    returnTest1Async(Ice.Current current)
    {
        var d1 = new D1();
        d1.sb = "D1.sb";
        d1.sd1 = "D1.sd1";
        var d2 = new D2();
        d2.pb = d1;
        d2.sb = "D2.sb";
        d2.sd2 = "D2.sd2";
        d2.pd2 = d1;
        d1.pb = d2;
        d1.pd1 = d2;
        return FromResult(((B?)d2, (B?)d2, (B?)d1));
    }

    public ValueTask<(B?, B?, B?)>
    returnTest2Async(Ice.Current current)
    {
        var d1 = new D1();
        d1.sb = "D1.sb";
        d1.sd1 = "D1.sd1";
        var d2 = new D2();
        d2.pb = d1;
        d2.sb = "D2.sb";
        d2.sd2 = "D2.sd2";
        d2.pd2 = d1;
        d1.pb = d2;
        d1.pd1 = d2;
        return FromResult< (B ?, B ?, B ?)>((d1, d1, d2));
    }

    public ValueTask<B?>
    returnTest3Async(B? p1, B? p2, Ice.Current current) => FromResult(p1);

    public ValueTask<SS3>
    sequenceTestAsync(SS1? p1, SS2? p2, Ice.Current current) => FromResult(new SS3(p1, p2));

    public ValueTask<(Dictionary<int, B?>, Dictionary<int, B?>)>
    dictionaryTestAsync(Dictionary<int, B?> bin, Ice.Current current)
    {
        var bout = new Dictionary<int, B?>();
        int i;
        for (i = 0; i < 10; ++i)
        {
            B? b = bin[i];
            TestHelper.Assert(b != null);
            var d2 = new D2();
            d2.sb = b.sb;
            d2.pb = b.pb;
            d2.sd2 = "D2";
            d2.pd2 = d2;
            bout[i * 10] = d2;
        }
        var r = new Dictionary<int, B?>();
        for (i = 0; i < 10; ++i)
        {
            string s = "D1." + (i * 20).ToString();
            var d1 = new D1();
            d1.sb = s;
            d1.pb = i == 0 ? null : r[(i - 1) * 20];
            d1.sd1 = s;
            d1.pd1 = d1;
            r[i * 20] = d1;
        }
        return FromResult((r, bout));
    }

    public ValueTask<PBase?>
    exchangePBaseAsync(PBase? pb, Ice.Current current) => FromResult(pb);

    public ValueTask<Preserved?>
    PBSUnknownAsPreservedAsync(Ice.Current current)
    {
        var r = new PSUnknown();
        r.pi = 5;
        r.ps = "preserved";
        r.psu = "unknown";
        r.graph = null;
        r.cl = new MyClass(15);
        return FromResult<Preserved?>(r);
    }

    public ValueTask
    checkPBSUnknownAsync(Preserved? p, Ice.Current current)
    {
        var pu = p as PSUnknown;
        TestHelper.Assert(pu != null);
        TestHelper.Assert(pu.pi == 5);
        TestHelper.Assert(pu.ps.Equals("preserved"));
        TestHelper.Assert(pu.psu.Equals("unknown"));
        TestHelper.Assert(pu.graph == null);
        TestHelper.Assert(pu.cl != null && pu.cl.i == 15);
        return new ValueTask(Task.CompletedTask);
    }

    public ValueTask<Preserved?>
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
        return FromResult<Preserved?>(r);
    }

    public ValueTask
    checkPBSUnknownWithGraphAsync(Preserved? p, Ice.Current current)
    {
        TestHelper.Assert(p is PSUnknown);
        var pu = (PSUnknown)p;
        TestHelper.Assert(pu.pi == 5);
        TestHelper.Assert(pu.ps.Equals("preserved"));
        TestHelper.Assert(pu.psu.Equals("unknown"));
        TestHelper.Assert(pu.graph != pu.graph!.next);
        TestHelper.Assert(pu.graph.next != pu.graph!.next!.next);
        TestHelper.Assert(pu.graph!.next!.next!.next == pu.graph);
        return new ValueTask(Task.CompletedTask);
    }

    public ValueTask<Preserved?>
    PBSUnknown2AsPreservedWithGraphAsync(Ice.Current current)
    {
        var r = new PSUnknown2();
        r.pi = 5;
        r.ps = "preserved";
        r.pb = r;
        return FromResult<Preserved?>(r);
    }

    public ValueTask
    checkPBSUnknown2WithGraphAsync(Preserved? p, Ice.Current current)
    {
        TestHelper.Assert(p is PSUnknown2);
        var pu = (PSUnknown2)p;
        TestHelper.Assert(pu.pi == 5);
        TestHelper.Assert(pu.ps.Equals("preserved"));
        TestHelper.Assert(pu.pb == pu);
        return new ValueTask(Task.CompletedTask);
    }

    public ValueTask<PNode?>
    exchangePNodeAsync(PNode? pn, Ice.Current current) => FromResult(pn);

    public ValueTask throwBaseAsBaseAsync(Ice.Current current)
    {
        var be = new BaseException();
        be.sbe = "sbe";
        be.pb = new B();
        be.pb.sb = "sb";
        be.pb.pb = be.pb;
        throw be;
    }

    public ValueTask throwDerivedAsBaseAsync(Ice.Current current)
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
        throw de;
    }

    public ValueTask
    throwDerivedAsDerivedAsync(Ice.Current current)
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
        throw de;
    }

    public ValueTask throwUnknownDerivedAsBaseAsync(Ice.Current current)
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

        throw ude;
    }

    public ValueTask throwPreservedExceptionAsync(Ice.Current current)
    {
        var ue = new PSUnknownException();
        ue.p = new PSUnknown2();
        ue.p.pi = 5;
        ue.p.ps = "preserved";
        ue.p.pb = ue.p;

        throw ue;
    }

    public ValueTask<Forward?>
    useForwardAsync(Ice.Current current)
    {
        var f = new Forward();
        f.h = new Hidden();
        f.h.f = f;
        return FromResult<Forward?>(f);
    }

    private static ValueTask<T> FromResult<T>(T result) => new ValueTask<T>(result);
}

public sealed class TestIntf2 : ITestIntf2
{
    public ValueTask<SBase?> SBSUnknownDerivedAsSBaseAsync(Ice.Current current)
        => new ValueTask<SBase?>(new SBSUnknownDerived("SBSUnknownDerived.sb", "SBSUnknownDerived.sbsud"));

    public ValueTask CUnknownAsSBaseAsync(SBase? cUnknown, Ice.Current current)
    {
        if (cUnknown!.sb != "CUnknown.sb")
        {
            throw new Exception();
        }
        return new ValueTask();
    }
}
