// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.slicing.exceptions;

import test.Ice.slicing.exceptions.server.Test.Base;
import test.Ice.slicing.exceptions.server.Test.KnownDerived;
import test.Ice.slicing.exceptions.server.Test.KnownIntermediate;
import test.Ice.slicing.exceptions.server.Test.KnownMostDerived;
import test.Ice.slicing.exceptions.server.Test.UnknownDerived;
import test.Ice.slicing.exceptions.server.Test.UnknownIntermediate;
import test.Ice.slicing.exceptions.server.Test.UnknownMostDerived1;
import test.Ice.slicing.exceptions.server.Test.UnknownMostDerived2;
import test.Ice.slicing.exceptions.server.Test._TestIntfDisp;

public final class TestI extends _TestIntfDisp
{
    public
    TestI()
    {
    }

    public void
    shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    public void
    baseAsBase(Ice.Current current)
        throws Base
    {
        Base b = new Base();
        b.b = "Base.b";
        throw b;
    }

    public void
    unknownDerivedAsBase(Ice.Current current)
        throws Base
    {
        UnknownDerived d = new UnknownDerived();
        d.b = "UnknownDerived.b";
        d.ud = "UnknownDerived.ud";
        throw d;
    }

    public void
    knownDerivedAsBase(Ice.Current current)
        throws Base
    {
        KnownDerived d = new KnownDerived();
        d.b = "KnownDerived.b";
        d.kd = "KnownDerived.kd";
        throw d;
    }

    public void
    knownDerivedAsKnownDerived(Ice.Current current)
        throws KnownDerived
    {
        KnownDerived d = new KnownDerived();
        d.b = "KnownDerived.b";
        d.kd = "KnownDerived.kd";
        throw d;
    }

    public void
    unknownIntermediateAsBase(Ice.Current current)
        throws Base
    {
        UnknownIntermediate ui = new UnknownIntermediate();
        ui.b = "UnknownIntermediate.b";
        ui.ui = "UnknownIntermediate.ui";
        throw ui;
    }

    public void
    knownIntermediateAsBase(Ice.Current current)
        throws Base
    {
        KnownIntermediate ki = new KnownIntermediate();
        ki.b = "KnownIntermediate.b";
        ki.ki = "KnownIntermediate.ki";
        throw ki;
    }

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

    public void
    knownIntermediateAsKnownIntermediate(Ice.Current current)
        throws KnownIntermediate
    {
        KnownIntermediate ki = new KnownIntermediate();
        ki.b = "KnownIntermediate.b";
        ki.ki = "KnownIntermediate.ki";
        throw ki;
    }

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
}
