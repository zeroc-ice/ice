// Copyright (c) ZeroC, Inc.

using Test;

public sealed class TestI : TestIntfDisp_
{
    public override void shutdown(Ice.Current current) => current.adapter.getCommunicator().shutdown();

    public override void baseAsBase(Ice.Current current)
    {
        var b = new Base();
        b.b = "Base.b";
        throw b;
    }

    public override void unknownDerivedAsBase(Ice.Current current)
    {
        var d = new UnknownDerived();
        d.b = "UnknownDerived.b";
        d.ud = "UnknownDerived.ud";
        throw d;
    }

    public override void knownDerivedAsBase(Ice.Current current)
    {
        var d = new KnownDerived();
        d.b = "KnownDerived.b";
        d.kd = "KnownDerived.kd";
        throw d;
    }

    public override void knownDerivedAsKnownDerived(Ice.Current current)
    {
        var d = new KnownDerived();
        d.b = "KnownDerived.b";
        d.kd = "KnownDerived.kd";
        throw d;
    }

    public override void unknownIntermediateAsBase(Ice.Current current)
    {
        var ui = new UnknownIntermediate();
        ui.b = "UnknownIntermediate.b";
        ui.ui = "UnknownIntermediate.ui";
        throw ui;
    }

    public override void knownIntermediateAsBase(Ice.Current current)
    {
        var ki = new KnownIntermediate();
        ki.b = "KnownIntermediate.b";
        ki.ki = "KnownIntermediate.ki";
        throw ki;
    }

    public override void knownMostDerivedAsBase(Ice.Current current)
    {
        var kmd = new KnownMostDerived();
        kmd.b = "KnownMostDerived.b";
        kmd.ki = "KnownMostDerived.ki";
        kmd.kmd = "KnownMostDerived.kmd";
        throw kmd;
    }

    public override void knownIntermediateAsKnownIntermediate(Ice.Current current)
    {
        var ki = new KnownIntermediate();
        ki.b = "KnownIntermediate.b";
        ki.ki = "KnownIntermediate.ki";
        throw ki;
    }

    public override void knownMostDerivedAsKnownIntermediate(Ice.Current current)
    {
        var kmd = new KnownMostDerived();
        kmd.b = "KnownMostDerived.b";
        kmd.ki = "KnownMostDerived.ki";
        kmd.kmd = "KnownMostDerived.kmd";
        throw kmd;
    }

    public override void knownMostDerivedAsKnownMostDerived(Ice.Current current)
    {
        var kmd = new KnownMostDerived();
        kmd.b = "KnownMostDerived.b";
        kmd.ki = "KnownMostDerived.ki";
        kmd.kmd = "KnownMostDerived.kmd";
        throw kmd;
    }

    public override void unknownMostDerived1AsBase(Ice.Current current)
    {
        var umd1 = new UnknownMostDerived1();
        umd1.b = "UnknownMostDerived1.b";
        umd1.ki = "UnknownMostDerived1.ki";
        umd1.umd1 = "UnknownMostDerived1.umd1";
        throw umd1;
    }

    public override void unknownMostDerived1AsKnownIntermediate(Ice.Current current)
    {
        var umd1 = new UnknownMostDerived1();
        umd1.b = "UnknownMostDerived1.b";
        umd1.ki = "UnknownMostDerived1.ki";
        umd1.umd1 = "UnknownMostDerived1.umd1";
        throw umd1;
    }

    public override void unknownMostDerived2AsBase(Ice.Current current)
    {
        var umd2 = new UnknownMostDerived2();
        umd2.b = "UnknownMostDerived2.b";
        umd2.ui = "UnknownMostDerived2.ui";
        umd2.umd2 = "UnknownMostDerived2.umd2";
        throw umd2;
    }
}
