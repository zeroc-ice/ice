// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.slicing.exceptions;

import test.Ice.slicing.exceptions.server.Test.*;

public final class TestI implements TestIntf
{
    private static void test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    @Override
    public void shutdown(com.zeroc.Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    @Override
    public void baseAsBase(com.zeroc.Ice.Current current)
        throws Base
    {
        Base b = new Base();
        b.b = "Base.b";
        throw b;
    }

    @Override
    public void unknownDerivedAsBase(com.zeroc.Ice.Current current)
        throws Base
    {
        UnknownDerived d = new UnknownDerived();
        d.b = "UnknownDerived.b";
        d.ud = "UnknownDerived.ud";
        throw d;
    }

    @Override
    public void knownDerivedAsBase(com.zeroc.Ice.Current current)
        throws Base
    {
        KnownDerived d = new KnownDerived();
        d.b = "KnownDerived.b";
        d.kd = "KnownDerived.kd";
        throw d;
    }

    @Override
    public void knownDerivedAsKnownDerived(com.zeroc.Ice.Current current)
        throws KnownDerived
    {
        KnownDerived d = new KnownDerived();
        d.b = "KnownDerived.b";
        d.kd = "KnownDerived.kd";
        throw d;
    }

    @Override
    public void unknownIntermediateAsBase(com.zeroc.Ice.Current current)
        throws Base
    {
        UnknownIntermediate ui = new UnknownIntermediate();
        ui.b = "UnknownIntermediate.b";
        ui.ui = "UnknownIntermediate.ui";
        throw ui;
    }

    @Override
    public void knownIntermediateAsBase(com.zeroc.Ice.Current current)
        throws Base
    {
        KnownIntermediate ki = new KnownIntermediate();
        ki.b = "KnownIntermediate.b";
        ki.ki = "KnownIntermediate.ki";
        throw ki;
    }

    @Override
    public void knownMostDerivedAsBase(com.zeroc.Ice.Current current)
        throws Base
    {
        KnownMostDerived kmd = new KnownMostDerived();
        kmd.b = "KnownMostDerived.b";
        kmd.ki = "KnownMostDerived.ki";
        kmd.kmd = "KnownMostDerived.kmd";
        throw kmd;
    }

    @Override
    public void knownIntermediateAsKnownIntermediate(com.zeroc.Ice.Current current)
        throws KnownIntermediate
    {
        KnownIntermediate ki = new KnownIntermediate();
        ki.b = "KnownIntermediate.b";
        ki.ki = "KnownIntermediate.ki";
        throw ki;
    }

    @Override
    public void knownMostDerivedAsKnownIntermediate(com.zeroc.Ice.Current current)
        throws KnownIntermediate
    {
        KnownMostDerived kmd = new KnownMostDerived();
        kmd.b = "KnownMostDerived.b";
        kmd.ki = "KnownMostDerived.ki";
        kmd.kmd = "KnownMostDerived.kmd";
        throw kmd;
    }

    @Override
    public void knownMostDerivedAsKnownMostDerived(com.zeroc.Ice.Current current)
        throws KnownMostDerived
    {
        KnownMostDerived kmd = new KnownMostDerived();
        kmd.b = "KnownMostDerived.b";
        kmd.ki = "KnownMostDerived.ki";
        kmd.kmd = "KnownMostDerived.kmd";
        throw kmd;
    }

    @Override
    public void
    unknownMostDerived1AsBase(com.zeroc.Ice.Current current)
        throws Base
    {
        UnknownMostDerived1 umd1 = new UnknownMostDerived1();
        umd1.b = "UnknownMostDerived1.b";
        umd1.ki = "UnknownMostDerived1.ki";
        umd1.umd1 = "UnknownMostDerived1.umd1";
        throw umd1;
    }

    @Override
    public void unknownMostDerived1AsKnownIntermediate(com.zeroc.Ice.Current current)
        throws KnownIntermediate
    {
        UnknownMostDerived1 umd1 = new UnknownMostDerived1();
        umd1.b = "UnknownMostDerived1.b";
        umd1.ki = "UnknownMostDerived1.ki";
        umd1.umd1 = "UnknownMostDerived1.umd1";
        throw umd1;
    }

    @Override
    public void unknownMostDerived2AsBase(com.zeroc.Ice.Current current)
        throws Base
    {
        UnknownMostDerived2 umd2 = new UnknownMostDerived2();
        umd2.b = "UnknownMostDerived2.b";
        umd2.ui = "UnknownMostDerived2.ui";
        umd2.umd2 = "UnknownMostDerived2.umd2";
        throw umd2;
    }

    @Override
    public void unknownMostDerived2AsBaseCompact(com.zeroc.Ice.Current current)
        throws Base
    {
        UnknownMostDerived2 umd2 = new UnknownMostDerived2();
        umd2.b = "UnknownMostDerived2.b";
        umd2.ui = "UnknownMostDerived2.ui";
        umd2.umd2 = "UnknownMostDerived2.umd2";
        throw umd2;
    }

    @Override
    public void knownPreservedAsBase(com.zeroc.Ice.Current current)
        throws Base
    {
        KnownPreservedDerived ex = new KnownPreservedDerived();
        ex.b = "base";
        ex.kp = "preserved";
        ex.kpd = "derived";
        throw ex;
    }

    @Override
    public void knownPreservedAsKnownPreserved(com.zeroc.Ice.Current current)
        throws KnownPreserved
    {
        KnownPreservedDerived ex = new KnownPreservedDerived();
        ex.b = "base";
        ex.kp = "preserved";
        ex.kpd = "derived";
        throw ex;
    }

    @Override
    public void relayKnownPreservedAsBase(RelayPrx r, com.zeroc.Ice.Current current)
        throws Base
    {
        RelayPrx p = RelayPrx.uncheckedCast(current.con.createProxy(r.ice_getIdentity()));
        p.knownPreservedAsBase();
        test(false);
    }

    @Override
    public void relayKnownPreservedAsKnownPreserved(RelayPrx r, com.zeroc.Ice.Current current)
        throws KnownPreserved
    {
        RelayPrx p = RelayPrx.uncheckedCast(current.con.createProxy(r.ice_getIdentity()));
        p.knownPreservedAsKnownPreserved();
        test(false);
    }

    @Override
    public void unknownPreservedAsBase(com.zeroc.Ice.Current current)
        throws Base
    {
        SPreserved2 ex = new SPreserved2();
        ex.b = "base";
        ex.kp = "preserved";
        ex.kpd = "derived";
        ex.p1 = new SPreservedClass("bc", "spc");
        ex.p2 = ex.p1;
        throw ex;
    }

    @Override
    public void unknownPreservedAsKnownPreserved(com.zeroc.Ice.Current current)
        throws KnownPreserved
    {
        SPreserved2 ex = new SPreserved2();
        ex.b = "base";
        ex.kp = "preserved";
        ex.kpd = "derived";
        ex.p1 = new SPreservedClass("bc", "spc");
        ex.p2 = ex.p1;
        throw ex;
    }

    @Override
    public void relayUnknownPreservedAsBase(RelayPrx r, com.zeroc.Ice.Current current)
        throws Base
    {
        RelayPrx p = RelayPrx.uncheckedCast(current.con.createProxy(r.ice_getIdentity()));
        p.unknownPreservedAsBase();
        test(false);
    }

    @Override
    public void relayUnknownPreservedAsKnownPreserved(RelayPrx r, com.zeroc.Ice.Current current)
        throws KnownPreserved
    {
        RelayPrx p = RelayPrx.uncheckedCast(current.con.createProxy(r.ice_getIdentity()));
        p.unknownPreservedAsKnownPreserved();
        test(false);
    }
}
