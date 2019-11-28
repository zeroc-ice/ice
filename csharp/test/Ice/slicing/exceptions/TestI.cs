//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

public sealed class TestI : TestIntf
{
    private static void test(bool b)
    {
        if (!b)
        {
            throw new System.Exception();
        }
    }

    public void shutdown(Ice.Current current)
    {
        current.adapter.GetCommunicator().shutdown();
    }

    public void baseAsBase(Ice.Current current)
    {
        Base b = new Base();
        b.b = "Base.b";
        throw b;
    }

    public void unknownDerivedAsBase(Ice.Current current)
    {
        UnknownDerived d = new UnknownDerived();
        d.b = "UnknownDerived.b";
        d.ud = "UnknownDerived.ud";
        throw d;
    }

    public void knownDerivedAsBase(Ice.Current current)
    {
        KnownDerived d = new KnownDerived();
        d.b = "KnownDerived.b";
        d.kd = "KnownDerived.kd";
        throw d;
    }

    public void knownDerivedAsKnownDerived(Ice.Current current)
    {
        KnownDerived d = new KnownDerived();
        d.b = "KnownDerived.b";
        d.kd = "KnownDerived.kd";
        throw d;
    }

    public void unknownIntermediateAsBase(Ice.Current current)
    {
        UnknownIntermediate ui = new UnknownIntermediate();
        ui.b = "UnknownIntermediate.b";
        ui.ui = "UnknownIntermediate.ui";
        throw ui;
    }

    public void knownIntermediateAsBase(Ice.Current current)
    {
        KnownIntermediate ki = new KnownIntermediate();
        ki.b = "KnownIntermediate.b";
        ki.ki = "KnownIntermediate.ki";
        throw ki;
    }

    public void knownMostDerivedAsBase(Ice.Current current)
    {
        KnownMostDerived kmd = new KnownMostDerived();
        kmd.b = "KnownMostDerived.b";
        kmd.ki = "KnownMostDerived.ki";
        kmd.kmd = "KnownMostDerived.kmd";
        throw kmd;
    }

    public void knownIntermediateAsKnownIntermediate(Ice.Current current)
    {
        KnownIntermediate ki = new KnownIntermediate();
        ki.b = "KnownIntermediate.b";
        ki.ki = "KnownIntermediate.ki";
        throw ki;
    }

    public void knownMostDerivedAsKnownIntermediate(Ice.Current current)
    {
        KnownMostDerived kmd = new KnownMostDerived();
        kmd.b = "KnownMostDerived.b";
        kmd.ki = "KnownMostDerived.ki";
        kmd.kmd = "KnownMostDerived.kmd";
        throw kmd;
    }

    public void knownMostDerivedAsKnownMostDerived(Ice.Current current)
    {
        KnownMostDerived kmd = new KnownMostDerived();
        kmd.b = "KnownMostDerived.b";
        kmd.ki = "KnownMostDerived.ki";
        kmd.kmd = "KnownMostDerived.kmd";
        throw kmd;
    }

    public void unknownMostDerived1AsBase(Ice.Current current)
    {
        UnknownMostDerived1 umd1 = new UnknownMostDerived1();
        umd1.b = "UnknownMostDerived1.b";
        umd1.ki = "UnknownMostDerived1.ki";
        umd1.umd1 = "UnknownMostDerived1.umd1";
        throw umd1;
    }

    public void unknownMostDerived1AsKnownIntermediate(Ice.Current current)
    {
        UnknownMostDerived1 umd1 = new UnknownMostDerived1();
        umd1.b = "UnknownMostDerived1.b";
        umd1.ki = "UnknownMostDerived1.ki";
        umd1.umd1 = "UnknownMostDerived1.umd1";
        throw umd1;
    }

    public void unknownMostDerived2AsBase(Ice.Current current)
    {
        UnknownMostDerived2 umd2 = new UnknownMostDerived2();
        umd2.b = "UnknownMostDerived2.b";
        umd2.ui = "UnknownMostDerived2.ui";
        umd2.umd2 = "UnknownMostDerived2.umd2";
        throw umd2;
    }

    public void unknownMostDerived2AsBaseCompact(Ice.Current current)
    {
        UnknownMostDerived2 umd2 = new UnknownMostDerived2();
        umd2.b = "UnknownMostDerived2.b";
        umd2.ui = "UnknownMostDerived2.ui";
        umd2.umd2 = "UnknownMostDerived2.umd2";
        throw umd2;
    }

    public void knownPreservedAsBase(Ice.Current current)
    {
        KnownPreservedDerived ex = new KnownPreservedDerived();
        ex.b = "base";
        ex.kp = "preserved";
        ex.kpd = "derived";
        throw ex;
    }

    public void knownPreservedAsKnownPreserved(Ice.Current current)
    {
        KnownPreservedDerived ex = new KnownPreservedDerived();
        ex.b = "base";
        ex.kp = "preserved";
        ex.kpd = "derived";
        throw ex;
    }

    public void relayKnownPreservedAsBase(RelayPrx r, Ice.Current current)
    {
        RelayPrx p = RelayPrx.UncheckedCast(current.con.createProxy(r.Identity));
        p.knownPreservedAsBase();
        test(false);
    }

    public void relayKnownPreservedAsKnownPreserved(RelayPrx r, Ice.Current current)
    {
        RelayPrx p = RelayPrx.UncheckedCast(current.con.createProxy(r.Identity));
        p.knownPreservedAsKnownPreserved();
        test(false);
    }

    public void unknownPreservedAsBase(Ice.Current current)
    {
        SPreserved2 ex = new SPreserved2();
        ex.b = "base";
        ex.kp = "preserved";
        ex.kpd = "derived";
        ex.p1 = new SPreservedClass("bc", "spc");
        ex.p2 = ex.p1;
        throw ex;
    }

    public void unknownPreservedAsKnownPreserved(Ice.Current current)
    {
        SPreserved2 ex = new SPreserved2();
        ex.b = "base";
        ex.kp = "preserved";
        ex.kpd = "derived";
        ex.p1 = new SPreservedClass("bc", "spc");
        ex.p2 = ex.p1;
        throw ex;
    }

    public void relayUnknownPreservedAsBase(RelayPrx r, Ice.Current current)
    {
        RelayPrx p = RelayPrx.UncheckedCast(current.con.createProxy(r.Identity));
        p.unknownPreservedAsBase();
        test(false);
    }

    public void relayUnknownPreservedAsKnownPreserved(RelayPrx r, Ice.Current current)
    {
        RelayPrx p = RelayPrx.UncheckedCast(current.con.createProxy(r.Identity));
        p.unknownPreservedAsKnownPreserved();
        test(false);
    }
}
