// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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

    public override void baseAsBase(Ice.Current current)
    {
        Base b = new Base();
        b.b = "Base.b";
        throw b;
    }

    public override void unknownDerivedAsBase(Ice.Current current)
    {
        UnknownDerived d = new UnknownDerived();
        d.b = "UnknownDerived.b";
        d.ud = "UnknownDerived.ud";
        throw d;
    }

    public override void knownDerivedAsBase(Ice.Current current)
    {
        KnownDerived d = new KnownDerived();
        d.b = "KnownDerived.b";
        d.kd = "KnownDerived.kd";
        throw d;
    }

    public override void knownDerivedAsKnownDerived(Ice.Current current)
    {
        KnownDerived d = new KnownDerived();
        d.b = "KnownDerived.b";
        d.kd = "KnownDerived.kd";
        throw d;
    }

    public override void unknownIntermediateAsBase(Ice.Current current)
    {
        UnknownIntermediate ui = new UnknownIntermediate();
        ui.b = "UnknownIntermediate.b";
        ui.ui = "UnknownIntermediate.ui";
        throw ui;
    }

    public override void knownIntermediateAsBase(Ice.Current current)
    {
        KnownIntermediate ki = new KnownIntermediate();
        ki.b = "KnownIntermediate.b";
        ki.ki = "KnownIntermediate.ki";
        throw ki;
    }

    public override void knownMostDerivedAsBase(Ice.Current current)
    {
        KnownMostDerived kmd = new KnownMostDerived();
        kmd.b = "KnownMostDerived.b";
        kmd.ki = "KnownMostDerived.ki";
        kmd.kmd = "KnownMostDerived.kmd";
        throw kmd;
    }

    public override void knownIntermediateAsKnownIntermediate(Ice.Current current)
    {
        KnownIntermediate ki = new KnownIntermediate();
        ki.b = "KnownIntermediate.b";
        ki.ki = "KnownIntermediate.ki";
        throw ki;
    }

    public override void knownMostDerivedAsKnownIntermediate(Ice.Current current)
    {
        KnownMostDerived kmd = new KnownMostDerived();
        kmd.b = "KnownMostDerived.b";
        kmd.ki = "KnownMostDerived.ki";
        kmd.kmd = "KnownMostDerived.kmd";
        throw kmd;
    }

    public override void knownMostDerivedAsKnownMostDerived(Ice.Current current)
    {
        KnownMostDerived kmd = new KnownMostDerived();
        kmd.b = "KnownMostDerived.b";
        kmd.ki = "KnownMostDerived.ki";
        kmd.kmd = "KnownMostDerived.kmd";
        throw kmd;
    }

    public override void unknownMostDerived1AsBase(Ice.Current current)
    {
        UnknownMostDerived1 umd1 = new UnknownMostDerived1();
        umd1.b = "UnknownMostDerived1.b";
        umd1.ki = "UnknownMostDerived1.ki";
        umd1.umd1 = "UnknownMostDerived1.umd1";
        throw umd1;
    }

    public override void unknownMostDerived1AsKnownIntermediate(Ice.Current current)
    {
        UnknownMostDerived1 umd1 = new UnknownMostDerived1();
        umd1.b = "UnknownMostDerived1.b";
        umd1.ki = "UnknownMostDerived1.ki";
        umd1.umd1 = "UnknownMostDerived1.umd1";
        throw umd1;
    }

    public override void unknownMostDerived2AsBase(Ice.Current current)
    {
        UnknownMostDerived2 umd2 = new UnknownMostDerived2();
        umd2.b = "UnknownMostDerived2.b";
        umd2.ui = "UnknownMostDerived2.ui";
        umd2.umd2 = "UnknownMostDerived2.umd2";
        throw umd2;
    }

    public override void unknownMostDerived2AsBaseCompact(Ice.Current current)
    {
        UnknownMostDerived2 umd2 = new UnknownMostDerived2();
        umd2.b = "UnknownMostDerived2.b";
        umd2.ui = "UnknownMostDerived2.ui";
        umd2.umd2 = "UnknownMostDerived2.umd2";
        throw umd2;
    }

    public override void knownPreservedAsBase(Ice.Current current)
    {
        KnownPreservedDerived ex = new KnownPreservedDerived();
        ex.b = "base";
        ex.kp = "preserved";
        ex.kpd = "derived";
        throw ex;
    }

    public override void knownPreservedAsKnownPreserved(Ice.Current current)
    {
        KnownPreservedDerived ex = new KnownPreservedDerived();
        ex.b = "base";
        ex.kp = "preserved";
        ex.kpd = "derived";
        throw ex;
    }

    public override void relayKnownPreservedAsBase(RelayPrx r, Ice.Current current)
    {
        r.knownPreservedAsBase();
        test(false);
    }

    public override void relayKnownPreservedAsKnownPreserved(RelayPrx r, Ice.Current current)
    {
        r.knownPreservedAsKnownPreserved();
        test(false);
    }

    public override void unknownPreservedAsBase(Ice.Current current)
    {
        SPreserved2 ex = new SPreserved2();
        ex.b = "base";
        ex.kp = "preserved";
        ex.kpd = "derived";
        ex.p1 = new SPreservedClass("bc", "spc");
        ex.p2 = ex.p1;
        throw ex;
    }

    public override void unknownPreservedAsKnownPreserved(Ice.Current current)
    {
        SPreserved2 ex = new SPreserved2();
        ex.b = "base";
        ex.kp = "preserved";
        ex.kpd = "derived";
        ex.p1 = new SPreservedClass("bc", "spc");
        ex.p2 = ex.p1;
        throw ex;
    }

    public override void relayUnknownPreservedAsBase(RelayPrx r, Ice.Current current)
    {
        r.unknownPreservedAsBase();
        test(false);
    }

    public override void relayUnknownPreservedAsKnownPreserved(RelayPrx r, Ice.Current current)
    {
        r.unknownPreservedAsKnownPreserved();
        test(false);
    }
}
