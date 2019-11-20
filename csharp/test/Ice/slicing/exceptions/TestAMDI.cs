//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Threading.Tasks;
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

    public Task shutdownAsync(Ice.Current current)
    {
        current.adapter.GetCommunicator().shutdown();
        return null;
    }

    public Task baseAsBaseAsync(Ice.Current current)
    {
        throw new Base("Base.b");
    }

    public Task unknownDerivedAsBaseAsync(Ice.Current current)
    {
        throw new UnknownDerived("UnknownDerived.b", "UnknownDerived.ud");
    }

    public Task knownDerivedAsBaseAsync(Ice.Current current)
    {
        throw new KnownDerived("KnownDerived.b", "KnownDerived.kd");
    }

    public Task
    knownDerivedAsKnownDerivedAsync(Ice.Current current)
    {
        throw new KnownDerived("KnownDerived.b", "KnownDerived.kd");
    }

    public Task
    unknownIntermediateAsBaseAsync(Ice.Current current)
    {
        throw new UnknownIntermediate("UnknownIntermediate.b", "UnknownIntermediate.ui");
    }

    public Task
    knownIntermediateAsBaseAsync(Ice.Current current)
    {
        throw new KnownIntermediate("KnownIntermediate.b", "KnownIntermediate.ki");
    }

    public Task
    knownMostDerivedAsBaseAsync(Ice.Current current)
    {
        throw new KnownMostDerived("KnownMostDerived.b", "KnownMostDerived.ki", "KnownMostDerived.kmd");
    }

    public Task
    knownIntermediateAsKnownIntermediateAsync(Ice.Current current)
    {
        throw new KnownIntermediate("KnownIntermediate.b", "KnownIntermediate.ki");
    }

    public Task
    knownMostDerivedAsKnownIntermediateAsync(Ice.Current current)
    {
        throw new KnownMostDerived("KnownMostDerived.b", "KnownMostDerived.ki", "KnownMostDerived.kmd");
    }

    public Task
    knownMostDerivedAsKnownMostDerivedAsync(Ice.Current current)
    {
        throw new KnownMostDerived("KnownMostDerived.b", "KnownMostDerived.ki", "KnownMostDerived.kmd");
    }

    public Task
    unknownMostDerived1AsBaseAsync(Ice.Current current)
    {
        throw new UnknownMostDerived1("UnknownMostDerived1.b", "UnknownMostDerived1.ki", "UnknownMostDerived1.umd1");
    }

    public Task
    unknownMostDerived1AsKnownIntermediateAsync(Ice.Current current)
    {
        throw new UnknownMostDerived1("UnknownMostDerived1.b", "UnknownMostDerived1.ki", "UnknownMostDerived1.umd1");
    }

    public Task
    unknownMostDerived2AsBaseAsync(Ice.Current current)
    {
        throw new UnknownMostDerived2("UnknownMostDerived2.b", "UnknownMostDerived2.ui", "UnknownMostDerived2.umd2");
    }

    public Task
    unknownMostDerived2AsBaseCompactAsync(Ice.Current current)
    {
        throw new UnknownMostDerived2("UnknownMostDerived2.b", "UnknownMostDerived2.ui", "UnknownMostDerived2.umd2");
    }

    public Task knownPreservedAsBaseAsync(Ice.Current current)
    {
        throw new KnownPreservedDerived("base", "preserved", "derived");
    }

    public Task
    knownPreservedAsKnownPreservedAsync(Ice.Current current)
    {
        throw new KnownPreservedDerived("base", "preserved", "derived");
    }

    public Task
    relayKnownPreservedAsBaseAsync(RelayPrx r, Ice.Current current)
    {
        RelayPrx p = RelayPrxHelper.uncheckedCast(current.con.createProxy(r.ice_getIdentity()));
        p.knownPreservedAsBase();
        test(false);
        return null;
    }

    public Task
    relayKnownPreservedAsKnownPreservedAsync(RelayPrx r, Ice.Current current)
    {
        RelayPrx p = RelayPrxHelper.uncheckedCast(current.con.createProxy(r.ice_getIdentity()));
        p.knownPreservedAsKnownPreserved();
        test(false);
        return null;
    }

    public Task unknownPreservedAsBaseAsync(Ice.Current current)
    {
        SPreserved2 ex = new SPreserved2();
        ex.b = "base";
        ex.kp = "preserved";
        ex.kpd = "derived";
        ex.p1 = new SPreservedClass("bc", "spc");
        ex.p2 = ex.p1;
        throw ex;
    }

    public Task
    unknownPreservedAsKnownPreservedAsync(Ice.Current current)
    {
        SPreserved2 ex = new SPreserved2();
        ex.b = "base";
        ex.kp = "preserved";
        ex.kpd = "derived";
        ex.p1 = new SPreservedClass("bc", "spc");
        ex.p2 = ex.p1;
        throw ex;
    }

    public Task
    relayUnknownPreservedAsBaseAsync(RelayPrx r, Ice.Current current)
    {
        RelayPrx p = RelayPrxHelper.uncheckedCast(current.con.createProxy(r.ice_getIdentity()));
        p.unknownPreservedAsBase();
        test(false);
        return null;
    }

    public Task
    relayUnknownPreservedAsKnownPreservedAsync(RelayPrx r, Ice.Current current)
    {
        RelayPrx p = RelayPrxHelper.uncheckedCast(current.con.createProxy(r.ice_getIdentity()));
        p.unknownPreservedAsKnownPreserved();
        test(false);
        return null;
    }
}
