// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


public final class TestI extends _TestDisp
{
    public
    TestI(Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;
    }

    public void
    shutdown_async(AMD_Test_shutdown cb, Ice.Current current)
    {
        _adapter.getCommunicator().shutdown();
	cb.ice_response();
    }

    public void
    baseAsBase_async(AMD_Test_baseAsBase cb, Ice.Current current)
        throws Base
    {
        Base b = new Base();
	b.b = "Base.b";
	cb.ice_exception(b);
    }

    public void
    unknownDerivedAsBase_async(AMD_Test_unknownDerivedAsBase cb, Ice.Current current)
        throws Base
    {
	UnknownDerived d = new UnknownDerived();
	d.b = "UnknownDerived.b";
	d.ud = "UnknownDerived.ud";
	cb.ice_exception(d);
    }

    public void
    knownDerivedAsBase_async(AMD_Test_knownDerivedAsBase cb, Ice.Current current)
        throws Base
    {
	KnownDerived d = new KnownDerived();
	d.b = "KnownDerived.b";
	d.kd = "KnownDerived.kd";
	cb.ice_exception(d);
    }

    public void
    knownDerivedAsKnownDerived_async(AMD_Test_knownDerivedAsKnownDerived cb, Ice.Current current)
        throws KnownDerived
    {
	KnownDerived d = new KnownDerived();
	d.b = "KnownDerived.b";
	d.kd = "KnownDerived.kd";
	cb.ice_exception(d);
    }

    public void
    unknownIntermediateAsBase_async(AMD_Test_unknownIntermediateAsBase cb, Ice.Current current)
        throws Base
    {
	UnknownIntermediate ui = new UnknownIntermediate();
	ui.b = "UnknownIntermediate.b";
	ui.ui = "UnknownIntermediate.ui";
	cb.ice_exception(ui);
    }

    public void
    knownIntermediateAsBase_async(AMD_Test_knownIntermediateAsBase cb, Ice.Current current)
        throws Base
    {
	KnownIntermediate ki = new KnownIntermediate();
	ki.b = "KnownIntermediate.b";
	ki.ki = "KnownIntermediate.ki";
	cb.ice_exception(ki);
    }

    public void
    knownMostDerivedAsBase_async(AMD_Test_knownMostDerivedAsBase cb, Ice.Current current)
        throws Base
    {
	KnownMostDerived kmd = new KnownMostDerived();
	kmd.b = "KnownMostDerived.b";
	kmd.ki = "KnownMostDerived.ki";
	kmd.kmd = "KnownMostDerived.kmd";
	cb.ice_exception(kmd);
    }

    public void
    knownIntermediateAsKnownIntermediate_async(AMD_Test_knownIntermediateAsKnownIntermediate cb, Ice.Current current)
        throws KnownIntermediate
    {
	KnownIntermediate ki = new KnownIntermediate();
	ki.b = "KnownIntermediate.b";
	ki.ki = "KnownIntermediate.ki";
	cb.ice_exception(ki);
    }

    public void
    knownMostDerivedAsKnownIntermediate_async(AMD_Test_knownMostDerivedAsKnownIntermediate cb, Ice.Current current)
        throws KnownIntermediate
    {
	KnownMostDerived kmd = new KnownMostDerived();
	kmd.b = "KnownMostDerived.b";
	kmd.ki = "KnownMostDerived.ki";
	kmd.kmd = "KnownMostDerived.kmd";
	cb.ice_exception(kmd);
    }

    public void
    knownMostDerivedAsKnownMostDerived_async(AMD_Test_knownMostDerivedAsKnownMostDerived cb, Ice.Current current)
        throws KnownMostDerived
    {
	KnownMostDerived kmd = new KnownMostDerived();
	kmd.b = "KnownMostDerived.b";
	kmd.ki = "KnownMostDerived.ki";
	kmd.kmd = "KnownMostDerived.kmd";
	cb.ice_exception(kmd);
    }

    public void
    unknownMostDerived1AsBase_async(AMD_Test_unknownMostDerived1AsBase cb, Ice.Current current)
        throws Base
    {
	UnknownMostDerived1 umd1 = new UnknownMostDerived1();
	umd1.b = "UnknownMostDerived1.b";
	umd1.ki = "UnknownMostDerived1.ki";
	umd1.umd1 = "UnknownMostDerived1.umd1";
	cb.ice_exception(umd1);
    }

    public void
    unknownMostDerived1AsKnownIntermediate_async(AMD_Test_unknownMostDerived1AsKnownIntermediate cb,
                                                 Ice.Current current)
        throws KnownIntermediate
    {
	UnknownMostDerived1 umd1 = new UnknownMostDerived1();
	umd1.b = "UnknownMostDerived1.b";
	umd1.ki = "UnknownMostDerived1.ki";
	umd1.umd1 = "UnknownMostDerived1.umd1";
	cb.ice_exception(umd1);
    }

    public void
    unknownMostDerived2AsBase_async(AMD_Test_unknownMostDerived2AsBase cb, Ice.Current current)
        throws Base
    {
	UnknownMostDerived2 umd2 = new UnknownMostDerived2();
	umd2.b = "UnknownMostDerived2.b";
	umd2.ui = "UnknownMostDerived2.ui";
	umd2.umd2 = "UnknownMostDerived2.umd2";
	cb.ice_exception(umd2);
    }
    private Ice.ObjectAdapter _adapter;
}
