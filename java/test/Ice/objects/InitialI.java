// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Test.*;

public final class InitialI extends Initial
{
    public
    InitialI(Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;
        _b1 = new BI();
        _b2 = new BI();
        _c = new CI();
        _d = new DI();

        _b1.theA = _b2; // Cyclic reference to another B
        _b1.theB = _b1; // Self reference.
        _b1.theC = null; // Null reference.

        _b2.theA = _b2; // Self reference, using base.
        _b2.theB = _b1; // Cyclic reference to another B
        _b2.theC = _c; // Cyclic reference to a C.

        _c.theB = _b2; // Cyclic reference to a B.

        _d.theA = _b1; // Reference to a B.
        _d.theB = _b2; // Reference to a B.
        _d.theC = null; // Reference to a C.
    }

    public void
    getAll(BHolder b1, BHolder b2, CHolder c, DHolder d, Ice.Current current)
    {
        _b1.preMarshalInvoked = false;
        _b2.preMarshalInvoked = false;
        _c.preMarshalInvoked = false;
        _d.preMarshalInvoked = false;
        b1.value = _b1;
        b2.value = _b2;
        c.value = _c;
        d.value = _d;
    }

    public B
    getB1(Ice.Current current)
    {
        _b1.preMarshalInvoked = false;
        _b2.preMarshalInvoked = false;
        _c.preMarshalInvoked = false;
        return _b1;
    }

    public B
    getB2(Ice.Current current)
    {
        _b1.preMarshalInvoked = false;
        _b2.preMarshalInvoked = false;
        _c.preMarshalInvoked = false;
        return _b2;
    }

    public C
    getC(Ice.Current current)
    {
        _b1.preMarshalInvoked = false;
        _b2.preMarshalInvoked = false;
        _c.preMarshalInvoked = false;
        return _c;
    }

    public D
    getD(Ice.Current current)
    {
        _b1.preMarshalInvoked = false;
        _b2.preMarshalInvoked = false;
        _c.preMarshalInvoked = false;
        _d.preMarshalInvoked = false;
        return _d;
    }

    public I
    getI(Ice.Current current)
    {
        return new II();
    }

    public I
    getJ(Ice.Current current)
    {
        return new JI();
    }

    public I
    getH(Ice.Current current)
    {
        return new HI();
    }

    public void
    setI(I theI, Ice.Current current)
    {
    }

    public void
    shutdown(Ice.Current current)
    {
        _adapter.getCommunicator().shutdown();
    }

    private Ice.ObjectAdapter _adapter;
    private B _b1;
    private B _b2;
    private C _c;
    private D _d;
}
