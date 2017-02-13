// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Threading.Tasks;
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

    public override Task shutdownAsync(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
        return null;
    }

    public override Task baseAsBaseAsync(Ice.Current current)
    {
        throw new Base("Base.b");
    }

    public override Task unknownDerivedAsBaseAsync(Ice.Current current)
    {
        throw new UnknownDerived("UnknownDerived.b", "UnknownDerived.ud");
    }

    public override Task knownDerivedAsBaseAsync(Ice.Current current)
    {
        throw new KnownDerived("KnownDerived.b", "KnownDerived.kd");
    }

    public override Task
    knownDerivedAsKnownDerivedAsync(Ice.Current current)
    {
        throw new KnownDerived("KnownDerived.b", "KnownDerived.kd");
    }

    public override Task
    unknownIntermediateAsBaseAsync(Ice.Current current)
    {
        throw new UnknownIntermediate("UnknownIntermediate.b", "UnknownIntermediate.ui");
    }

    public override Task
    knownIntermediateAsBaseAsync(Ice.Current current)
    {
        throw new KnownIntermediate("KnownIntermediate.b", "KnownIntermediate.ki");
    }

    public override Task
    knownMostDerivedAsBaseAsync(Ice.Current current)
    {
        throw new KnownMostDerived("KnownMostDerived.b", "KnownMostDerived.ki", "KnownMostDerived.kmd");
    }

    public override Task
    knownIntermediateAsKnownIntermediateAsync(Ice.Current current)
    {
        throw new KnownIntermediate("KnownIntermediate.b", "KnownIntermediate.ki");
    }

    public override Task
    knownMostDerivedAsKnownIntermediateAsync(Ice.Current current)
    {
        throw new KnownMostDerived("KnownMostDerived.b", "KnownMostDerived.ki", "KnownMostDerived.kmd");
    }

    public override Task
    knownMostDerivedAsKnownMostDerivedAsync(Ice.Current current)
    {
        throw new KnownMostDerived("KnownMostDerived.b", "KnownMostDerived.ki", "KnownMostDerived.kmd");
    }

    public override Task
    unknownMostDerived1AsBaseAsync(Ice.Current current)
    {
        throw new UnknownMostDerived1("UnknownMostDerived1.b", "UnknownMostDerived1.ki", "UnknownMostDerived1.umd1");
    }

    public override Task
    unknownMostDerived1AsKnownIntermediateAsync(Ice.Current current)
    {
        throw new UnknownMostDerived1("UnknownMostDerived1.b", "UnknownMostDerived1.ki", "UnknownMostDerived1.umd1");
    }

    public override Task
    unknownMostDerived2AsBaseAsync(Ice.Current current)
    {
        throw new UnknownMostDerived2("UnknownMostDerived2.b", "UnknownMostDerived2.ui", "UnknownMostDerived2.umd2");
    }

    public override Task
    unknownMostDerived2AsBaseCompactAsync(Ice.Current current)
    {
        throw new UnknownMostDerived2("UnknownMostDerived2.b", "UnknownMostDerived2.ui", "UnknownMostDerived2.umd2");
    }

    public override Task knownPreservedAsBaseAsync(Ice.Current current)
    {
        throw new KnownPreservedDerived("base", "preserved", "derived");
    }

    public override Task
    knownPreservedAsKnownPreservedAsync(Ice.Current current)
    {
        throw new KnownPreservedDerived("base", "preserved", "derived");
    }

    public override Task
    relayKnownPreservedAsBaseAsync(RelayPrx r, Ice.Current current)
    {
        r.knownPreservedAsBase();
        test(false);
        return null;
    }

    public override Task
    relayKnownPreservedAsKnownPreservedAsync(RelayPrx r, Ice.Current current)
    {
        r.knownPreservedAsKnownPreserved();
        test(false);
        return null;
    }

    public override Task unknownPreservedAsBaseAsync(Ice.Current current)
    {
        SPreserved2 ex = new SPreserved2();
        ex.b = "base";
        ex.kp = "preserved";
        ex.kpd = "derived";
        ex.p1 = new SPreservedClass("bc", "spc");
        ex.p2 = ex.p1;
        throw ex;
    }

    public override Task
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

    public override Task
    relayUnknownPreservedAsBaseAsync(RelayPrx r, Ice.Current current)
    {
        r.unknownPreservedAsBase();
        test(false);
        return null;
    }

    public override Task
    relayUnknownPreservedAsKnownPreservedAsync(RelayPrx r, Ice.Current current)
    {
        r.unknownPreservedAsKnownPreserved();
        test(false);
        return null;
    }
}
