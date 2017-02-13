// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
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

    public override void shutdown_async(AMD_TestIntf_shutdown cb, Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
        cb.ice_response();
    }

    public override void baseAsBase_async(AMD_TestIntf_baseAsBase cb, Ice.Current current)
    {
        Base b = new Base();
        b.b = "Base.b";
        cb.ice_exception(b);
    }

    public override void unknownDerivedAsBase_async(AMD_TestIntf_unknownDerivedAsBase cb, Ice.Current current)
    {
        UnknownDerived d = new UnknownDerived();
        d.b = "UnknownDerived.b";
        d.ud = "UnknownDerived.ud";
        cb.ice_exception(d);
    }

    public override void knownDerivedAsBase_async(AMD_TestIntf_knownDerivedAsBase cb, Ice.Current current)
    {
        KnownDerived d = new KnownDerived();
        d.b = "KnownDerived.b";
        d.kd = "KnownDerived.kd";
        cb.ice_exception(d);
    }

    public override void knownDerivedAsKnownDerived_async(AMD_TestIntf_knownDerivedAsKnownDerived cb,
                                                          Ice.Current current)
    {
        KnownDerived d = new KnownDerived();
        d.b = "KnownDerived.b";
        d.kd = "KnownDerived.kd";
        cb.ice_exception(d);
    }

    public override void unknownIntermediateAsBase_async(AMD_TestIntf_unknownIntermediateAsBase cb, Ice.Current current)
    {
        UnknownIntermediate ui = new UnknownIntermediate();
        ui.b = "UnknownIntermediate.b";
        ui.ui = "UnknownIntermediate.ui";
        cb.ice_exception(ui);
    }

    public override void knownIntermediateAsBase_async(AMD_TestIntf_knownIntermediateAsBase cb, Ice.Current current)
    {
        KnownIntermediate ki = new KnownIntermediate();
        ki.b = "KnownIntermediate.b";
        ki.ki = "KnownIntermediate.ki";
        cb.ice_exception(ki);
    }

    public override void knownMostDerivedAsBase_async(AMD_TestIntf_knownMostDerivedAsBase cb, Ice.Current current)
    {
        KnownMostDerived kmd = new KnownMostDerived();
        kmd.b = "KnownMostDerived.b";
        kmd.ki = "KnownMostDerived.ki";
        kmd.kmd = "KnownMostDerived.kmd";
        cb.ice_exception(kmd);
    }

    public override void knownIntermediateAsKnownIntermediate_async(
        AMD_TestIntf_knownIntermediateAsKnownIntermediate cb, Ice.Current current)
    {
        KnownIntermediate ki = new KnownIntermediate();
        ki.b = "KnownIntermediate.b";
        ki.ki = "KnownIntermediate.ki";
        cb.ice_exception(ki);
    }

    public override void knownMostDerivedAsKnownIntermediate_async(AMD_TestIntf_knownMostDerivedAsKnownIntermediate cb,
                                                                   Ice.Current current)
    {
        KnownMostDerived kmd = new KnownMostDerived();
        kmd.b = "KnownMostDerived.b";
        kmd.ki = "KnownMostDerived.ki";
        kmd.kmd = "KnownMostDerived.kmd";
        cb.ice_exception(kmd);
    }

    public override void knownMostDerivedAsKnownMostDerived_async(AMD_TestIntf_knownMostDerivedAsKnownMostDerived cb,
                                                                  Ice.Current current)
    {
        KnownMostDerived kmd = new KnownMostDerived();
        kmd.b = "KnownMostDerived.b";
        kmd.ki = "KnownMostDerived.ki";
        kmd.kmd = "KnownMostDerived.kmd";
        cb.ice_exception(kmd);
    }

    public override void unknownMostDerived1AsBase_async(AMD_TestIntf_unknownMostDerived1AsBase cb, Ice.Current current)
    {
        UnknownMostDerived1 umd1 = new UnknownMostDerived1();
        umd1.b = "UnknownMostDerived1.b";
        umd1.ki = "UnknownMostDerived1.ki";
        umd1.umd1 = "UnknownMostDerived1.umd1";
        cb.ice_exception(umd1);
    }

    public override void unknownMostDerived1AsKnownIntermediate_async(
        AMD_TestIntf_unknownMostDerived1AsKnownIntermediate cb, Ice.Current current)
    {
        UnknownMostDerived1 umd1 = new UnknownMostDerived1();
        umd1.b = "UnknownMostDerived1.b";
        umd1.ki = "UnknownMostDerived1.ki";
        umd1.umd1 = "UnknownMostDerived1.umd1";
        cb.ice_exception(umd1);
    }

    public override void unknownMostDerived2AsBase_async(AMD_TestIntf_unknownMostDerived2AsBase cb, Ice.Current current)
    {
        UnknownMostDerived2 umd2 = new UnknownMostDerived2();
        umd2.b = "UnknownMostDerived2.b";
        umd2.ui = "UnknownMostDerived2.ui";
        umd2.umd2 = "UnknownMostDerived2.umd2";
        cb.ice_exception(umd2);
    }

    public override void unknownMostDerived2AsBaseCompact_async(AMD_TestIntf_unknownMostDerived2AsBaseCompact cb,
                                                                Ice.Current current)
    {
        UnknownMostDerived2 umd2 = new UnknownMostDerived2();
        umd2.b = "UnknownMostDerived2.b";
        umd2.ui = "UnknownMostDerived2.ui";
        umd2.umd2 = "UnknownMostDerived2.umd2";
        cb.ice_exception(umd2);
    }

    public override void knownPreservedAsBase_async(AMD_TestIntf_knownPreservedAsBase cb, Ice.Current current)
    {
        KnownPreservedDerived ex = new KnownPreservedDerived();
        ex.b = "base";
        ex.kp = "preserved";
        ex.kpd = "derived";
        cb.ice_exception(ex);
    }

    public override void knownPreservedAsKnownPreserved_async(AMD_TestIntf_knownPreservedAsKnownPreserved cb,
                                                              Ice.Current current)
    {
        KnownPreservedDerived ex = new KnownPreservedDerived();
        ex.b = "base";
        ex.kp = "preserved";
        ex.kpd = "derived";
        cb.ice_exception(ex);
    }

    public override void relayKnownPreservedAsBase_async(AMD_TestIntf_relayKnownPreservedAsBase cb,
                                                         RelayPrx r, Ice.Current current)
    {
        try
        {
            r.knownPreservedAsBase();
            test(false);
        }
        catch(Ice.UserException ex)
        {
            cb.ice_exception(ex);
        }
        catch(Ice.LocalException ex)
        {
            cb.ice_exception(ex);
        }
    }

    public override void relayKnownPreservedAsKnownPreserved_async(AMD_TestIntf_relayKnownPreservedAsKnownPreserved cb,
                                                                   RelayPrx r, Ice.Current current)
    {
        try
        {
            r.knownPreservedAsKnownPreserved();
            test(false);
        }
        catch(Ice.UserException ex)
        {
            cb.ice_exception(ex);
        }
        catch(Ice.LocalException ex)
        {
            cb.ice_exception(ex);
        }
    }

    public override void unknownPreservedAsBase_async(AMD_TestIntf_unknownPreservedAsBase cb, Ice.Current current)
    {
        SPreserved2 ex = new SPreserved2();
        ex.b = "base";
        ex.kp = "preserved";
        ex.kpd = "derived";
        ex.p1 = new SPreservedClass("bc", "spc");
        ex.p2 = ex.p1;
        cb.ice_exception(ex);
    }

    public override void unknownPreservedAsKnownPreserved_async(AMD_TestIntf_unknownPreservedAsKnownPreserved cb,
                                                                Ice.Current current)
    {
        SPreserved2 ex = new SPreserved2();
        ex.b = "base";
        ex.kp = "preserved";
        ex.kpd = "derived";
        ex.p1 = new SPreservedClass("bc", "spc");
        ex.p2 = ex.p1;
        cb.ice_exception(ex);
    }

    public override void relayUnknownPreservedAsBase_async(AMD_TestIntf_relayUnknownPreservedAsBase cb,
                                                           RelayPrx r, Ice.Current current)
    {
        try
        {
            r.unknownPreservedAsBase();
            test(false);
        }
        catch(Ice.UserException ex)
        {
            cb.ice_exception(ex);
        }
        catch(Ice.LocalException ex)
        {
            cb.ice_exception(ex);
        }
    }

    public override void relayUnknownPreservedAsKnownPreserved_async(
        AMD_TestIntf_relayUnknownPreservedAsKnownPreserved cb, RelayPrx r, Ice.Current current)
    {
        try
        {
            r.unknownPreservedAsKnownPreserved();
            test(false);
        }
        catch(Ice.UserException ex)
        {
            cb.ice_exception(ex);
        }
        catch(Ice.LocalException ex)
        {
            cb.ice_exception(ex);
        }
    }
}
