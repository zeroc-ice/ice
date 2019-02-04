//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.slicing.exceptions;

import test.Ice.slicing.exceptions.server.Test.*;

public final class TestI extends _TestIntfDisp
{
    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    @Override
    public void
    shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    @Override
    public void
    baseAsBase(Ice.Current current)
        throws Base
    {
        Base b = new Base();
        b.b = "Base.b";
        throw b;
    }

    @Override
    public void
    unknownDerivedAsBase(Ice.Current current)
        throws Base
    {
        UnknownDerived d = new UnknownDerived();
        d.b = "UnknownDerived.b";
        d.ud = "UnknownDerived.ud";
        throw d;
    }

    @Override
    public void
    knownDerivedAsBase(Ice.Current current)
        throws Base
    {
        KnownDerived d = new KnownDerived();
        d.b = "KnownDerived.b";
        d.kd = "KnownDerived.kd";
        throw d;
    }

    @Override
    public void
    knownDerivedAsKnownDerived(Ice.Current current)
        throws KnownDerived
    {
        KnownDerived d = new KnownDerived();
        d.b = "KnownDerived.b";
        d.kd = "KnownDerived.kd";
        throw d;
    }

    @Override
    public void
    unknownIntermediateAsBase(Ice.Current current)
        throws Base
    {
        UnknownIntermediate ui = new UnknownIntermediate();
        ui.b = "UnknownIntermediate.b";
        ui.ui = "UnknownIntermediate.ui";
        throw ui;
    }

    @Override
    public void
    knownIntermediateAsBase(Ice.Current current)
        throws Base
    {
        KnownIntermediate ki = new KnownIntermediate();
        ki.b = "KnownIntermediate.b";
        ki.ki = "KnownIntermediate.ki";
        throw ki;
    }

    @Override
    public void
    knownMostDerivedAsBase(Ice.Current current)
        throws Base
    {
        KnownMostDerived kmd = new KnownMostDerived();
        kmd.b = "KnownMostDerived.b";
        kmd.ki = "KnownMostDerived.ki";
        kmd.kmd = "KnownMostDerived.kmd";
        throw kmd;
    }

    @Override
    public void
    knownIntermediateAsKnownIntermediate(Ice.Current current)
        throws KnownIntermediate
    {
        KnownIntermediate ki = new KnownIntermediate();
        ki.b = "KnownIntermediate.b";
        ki.ki = "KnownIntermediate.ki";
        throw ki;
    }

    @Override
    public void
    knownMostDerivedAsKnownIntermediate(Ice.Current current)
        throws KnownIntermediate
    {
        KnownMostDerived kmd = new KnownMostDerived();
        kmd.b = "KnownMostDerived.b";
        kmd.ki = "KnownMostDerived.ki";
        kmd.kmd = "KnownMostDerived.kmd";
        throw kmd;
    }

    @Override
    public void
    knownMostDerivedAsKnownMostDerived(Ice.Current current)
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
    unknownMostDerived1AsBase(Ice.Current current)
        throws Base
    {
        UnknownMostDerived1 umd1 = new UnknownMostDerived1();
        umd1.b = "UnknownMostDerived1.b";
        umd1.ki = "UnknownMostDerived1.ki";
        umd1.umd1 = "UnknownMostDerived1.umd1";
        throw umd1;
    }

    @Override
    public void
    unknownMostDerived1AsKnownIntermediate(Ice.Current current)
        throws KnownIntermediate
    {
        UnknownMostDerived1 umd1 = new UnknownMostDerived1();
        umd1.b = "UnknownMostDerived1.b";
        umd1.ki = "UnknownMostDerived1.ki";
        umd1.umd1 = "UnknownMostDerived1.umd1";
        throw umd1;
    }

    @Override
    public void
    unknownMostDerived2AsBase(Ice.Current current)
        throws Base
    {
        UnknownMostDerived2 umd2 = new UnknownMostDerived2();
        umd2.b = "UnknownMostDerived2.b";
        umd2.ui = "UnknownMostDerived2.ui";
        umd2.umd2 = "UnknownMostDerived2.umd2";
        throw umd2;
    }

    @Override
    public void
    unknownMostDerived2AsBaseCompact(Ice.Current current)
        throws Base
    {
        UnknownMostDerived2 umd2 = new UnknownMostDerived2();
        umd2.b = "UnknownMostDerived2.b";
        umd2.ui = "UnknownMostDerived2.ui";
        umd2.umd2 = "UnknownMostDerived2.umd2";
        throw umd2;
    }

    @Override
    public void
    knownPreservedAsBase(Ice.Current current)
        throws Base
    {
        KnownPreservedDerived ex = new KnownPreservedDerived();
        ex.b = "base";
        ex.kp = "preserved";
        ex.kpd = "derived";
        throw ex;
    }

    @Override
    public void
    knownPreservedAsKnownPreserved(Ice.Current current)
        throws KnownPreserved
    {
        KnownPreservedDerived ex = new KnownPreservedDerived();
        ex.b = "base";
        ex.kp = "preserved";
        ex.kpd = "derived";
        throw ex;
    }

    @Override
    public void
    relayKnownPreservedAsBase(RelayPrx r, Ice.Current current)
        throws Base
    {
        RelayPrx p = RelayPrxHelper.uncheckedCast(current.con.createProxy(r.ice_getIdentity()));
        p.knownPreservedAsBase();
        test(false);
    }

    @Override
    public void
    relayKnownPreservedAsKnownPreserved(RelayPrx r, Ice.Current current)
        throws KnownPreserved
    {
        RelayPrx p = RelayPrxHelper.uncheckedCast(current.con.createProxy(r.ice_getIdentity()));
        p.knownPreservedAsKnownPreserved();
        test(false);
    }

    @Override
    public void
    unknownPreservedAsBase(Ice.Current current)
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
    public void
    unknownPreservedAsKnownPreserved(Ice.Current current)
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
    public void
    relayUnknownPreservedAsBase(RelayPrx r, Ice.Current current)
        throws Base
    {
        RelayPrx p = RelayPrxHelper.uncheckedCast(current.con.createProxy(r.ice_getIdentity()));
        p.unknownPreservedAsBase();
        test(false);
    }

    @Override
    public void
    relayUnknownPreservedAsKnownPreserved(RelayPrx r, Ice.Current current)
        throws KnownPreserved
    {
        RelayPrx p = RelayPrxHelper.uncheckedCast(current.con.createProxy(r.ice_getIdentity()));
        p.unknownPreservedAsKnownPreserved();
        test(false);
    }
}
