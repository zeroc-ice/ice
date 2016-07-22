// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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

    public override void shutdownAsync(Action response, Action<Exception> exception,Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
        response();
    }

    public override void baseAsBaseAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        exception(new Base("Base.b"));
    }

    public override void unknownDerivedAsBaseAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        exception(new UnknownDerived("UnknownDerived.b", "UnknownDerived.ud"));
    }

    public override void knownDerivedAsBaseAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        exception(new KnownDerived("KnownDerived.b", "KnownDerived.kd"));
    }

    public override void
    knownDerivedAsKnownDerivedAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        exception(new KnownDerived("KnownDerived.b", "KnownDerived.kd"));
    }

    public override void
    unknownIntermediateAsBaseAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        exception(new UnknownIntermediate("UnknownIntermediate.b", "UnknownIntermediate.ui"));
    }

    public override void
    knownIntermediateAsBaseAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        exception(new KnownIntermediate("KnownIntermediate.b", "KnownIntermediate.ki"));
    }

    public override void
    knownMostDerivedAsBaseAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        exception(new KnownMostDerived("KnownMostDerived.b", "KnownMostDerived.ki", "KnownMostDerived.kmd"));
    }

    public override void
    knownIntermediateAsKnownIntermediateAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        exception(new KnownIntermediate("KnownIntermediate.b", "KnownIntermediate.ki"));
    }

    public override void
    knownMostDerivedAsKnownIntermediateAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        exception(new KnownMostDerived("KnownMostDerived.b", "KnownMostDerived.ki", "KnownMostDerived.kmd"));
    }

    public override void
    knownMostDerivedAsKnownMostDerivedAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        exception(new KnownMostDerived("KnownMostDerived.b", "KnownMostDerived.ki", "KnownMostDerived.kmd"));
    }

    public override void
    unknownMostDerived1AsBaseAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        exception(new UnknownMostDerived1("UnknownMostDerived1.b", "UnknownMostDerived1.ki", 
                                          "UnknownMostDerived1.umd1"));
    }

    public override void
    unknownMostDerived1AsKnownIntermediateAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        exception(new UnknownMostDerived1("UnknownMostDerived1.b", "UnknownMostDerived1.ki",
                                          "UnknownMostDerived1.umd1"));
    }

    public override void
    unknownMostDerived2AsBaseAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        exception(new UnknownMostDerived2("UnknownMostDerived2.b", "UnknownMostDerived2.ui",
                                          "UnknownMostDerived2.umd2"));
    }

    public override void
    unknownMostDerived2AsBaseCompactAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        exception(new UnknownMostDerived2("UnknownMostDerived2.b", "UnknownMostDerived2.ui",
                                          "UnknownMostDerived2.umd2"));
    }

    public override void knownPreservedAsBaseAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        exception(new KnownPreservedDerived("base", "preserved", "derived"));
    }

    public override void
    knownPreservedAsKnownPreservedAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        exception(new KnownPreservedDerived("base", "preserved", "derived"));
    }

    public override void
    relayKnownPreservedAsBaseAsync(RelayPrx r, Action response, Action<Exception> exception, Ice.Current current)
    {
        try
        {
            r.knownPreservedAsBase();
            test(false);
        }
        catch(Ice.UserException ex)
        {
            exception(ex);
        }
        catch(Ice.LocalException ex)
        {
            exception(ex);
        }
    }

    public override void
    relayKnownPreservedAsKnownPreservedAsync(RelayPrx r, Action response, Action<Exception> exception,
                                             Ice.Current current)
    {
        try
        {
            r.knownPreservedAsKnownPreserved();
            test(false);
        }
        catch(Ice.UserException ex)
        {
            exception(ex);
        }
        catch(Ice.LocalException ex)
        {
            exception(ex);
        }
    }

    public override void unknownPreservedAsBaseAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        SPreserved2 ex = new SPreserved2();
        ex.b = "base";
        ex.kp = "preserved";
        ex.kpd = "derived";
        ex.p1 = new SPreservedClass("bc", "spc");
        ex.p2 = ex.p1;
        exception(ex);
    }

    public override void
    unknownPreservedAsKnownPreservedAsync(Action response, Action<Exception> exception, Ice.Current current)
    {
        SPreserved2 ex = new SPreserved2();
        ex.b = "base";
        ex.kp = "preserved";
        ex.kpd = "derived";
        ex.p1 = new SPreservedClass("bc", "spc");
        ex.p2 = ex.p1;
        exception(ex);
    }

    public override void
    relayUnknownPreservedAsBaseAsync(RelayPrx r, Action response, Action<Exception> exception, Ice.Current current)
    {
        try
        {
            r.unknownPreservedAsBase();
            test(false);
        }
        catch(Ice.UserException ex)
        {
            exception(ex);
        }
        catch(Ice.LocalException ex)
        {
            exception(ex);
        }
    }

    public override void
    relayUnknownPreservedAsKnownPreservedAsync(RelayPrx r, Action response, Action<Exception> exception,
                                               Ice.Current current)
    {
        try
        {
            r.unknownPreservedAsKnownPreserved();
            test(false);
        }
        catch(Ice.UserException ex)
        {
            exception(ex);
        }
        catch(Ice.LocalException ex)
        {
            exception(ex);
        }

    }
}
