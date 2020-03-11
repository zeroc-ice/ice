//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice;
using System;
using System.Diagnostics;
using System.Threading.Tasks;
using Test;

public sealed class TestIntf : ITestIntf
{
    private static void test(bool b)
    {
        if (!b)
        {
            throw new System.Exception();
        }
    }

    public ValueTask shutdownAsync(Ice.Current current)
    {
        current.Adapter.Communicator.Shutdown();
        return new ValueTask(Task.CompletedTask);
    }

    public ValueTask baseAsBaseAsync(Ice.Current current) => throw new Base("Base.b");

    public ValueTask unknownDerivedAsBaseAsync(Ice.Current current) =>
        throw new UnknownDerived("UnknownDerived.b", "UnknownDerived.ud");

    public ValueTask knownDerivedAsBaseAsync(Ice.Current current) =>
        throw new KnownDerived("KnownDerived.b", "KnownDerived.kd");

    public ValueTask
    knownDerivedAsKnownDerivedAsync(Ice.Current current) =>
        throw new KnownDerived("KnownDerived.b", "KnownDerived.kd");

    public ValueTask
    unknownIntermediateAsBaseAsync(Ice.Current current) =>
        throw new UnknownIntermediate("UnknownIntermediate.b", "UnknownIntermediate.ui");

    public ValueTask
    knownIntermediateAsBaseAsync(Ice.Current current) =>
        throw new KnownIntermediate("KnownIntermediate.b", "KnownIntermediate.ki");

    public ValueTask
    knownMostDerivedAsBaseAsync(Ice.Current current)
    {
        throw new KnownMostDerived("KnownMostDerived.b", "KnownMostDerived.ki", "KnownMostDerived.kmd");
    }

    public ValueTask
    knownIntermediateAsKnownIntermediateAsync(Ice.Current current) =>
        throw new KnownIntermediate("KnownIntermediate.b", "KnownIntermediate.ki");

    public ValueTask
    knownMostDerivedAsKnownIntermediateAsync(Ice.Current current) =>
        throw new KnownMostDerived("KnownMostDerived.b", "KnownMostDerived.ki", "KnownMostDerived.kmd");

    public ValueTask
    knownMostDerivedAsKnownMostDerivedAsync(Ice.Current current) =>
        throw new KnownMostDerived("KnownMostDerived.b", "KnownMostDerived.ki", "KnownMostDerived.kmd");

    public ValueTask
    unknownMostDerived1AsBaseAsync(Ice.Current current) =>
        throw new UnknownMostDerived1("UnknownMostDerived1.b", "UnknownMostDerived1.ki", "UnknownMostDerived1.umd1");

    public ValueTask
    unknownMostDerived1AsKnownIntermediateAsync(Ice.Current current) =>
        throw new UnknownMostDerived1("UnknownMostDerived1.b", "UnknownMostDerived1.ki", "UnknownMostDerived1.umd1");

    public ValueTask
    unknownMostDerived2AsBaseAsync(Ice.Current current) =>
        throw new UnknownMostDerived2("UnknownMostDerived2.b", "UnknownMostDerived2.ui", "UnknownMostDerived2.umd2");

    public ValueTask
    unknownMostDerived2AsBaseCompactAsync(Ice.Current current) =>
        throw new UnknownMostDerived2("UnknownMostDerived2.b", "UnknownMostDerived2.ui", "UnknownMostDerived2.umd2");

    public ValueTask knownPreservedAsBaseAsync(Ice.Current current) =>
        throw new KnownPreservedDerived("base", "preserved", "derived");

    public ValueTask
    knownPreservedAsKnownPreservedAsync(Ice.Current current) =>
        throw new KnownPreservedDerived("base", "preserved", "derived");

    public ValueTask serverPrivateExceptionAsync(Current current) => throw new ServerPrivateException("ServerPrivate");

    public ValueTask
    relayKnownPreservedAsBaseAsync(IRelayPrx? r, Ice.Current current)
    {
        Debug.Assert(r != null);
        IRelayPrx p = current.Connection.CreateProxy(r.Identity, IRelayPrx.Factory);
        try
        {
            p.knownPreservedAsBase();
        }
        catch (RemoteException ex)
        {
            test(ex.ConvertToUnhandled);
            ex.ConvertToUnhandled = false;
            throw;
        }
        test(false);
        return new ValueTask(Task.CompletedTask);
    }

    public ValueTask
    relayKnownPreservedAsKnownPreservedAsync(IRelayPrx? r, Ice.Current current)
    {
        Debug.Assert(r != null);
        IRelayPrx p = current.Connection.CreateProxy(r.Identity, IRelayPrx.Factory);
        try
        {
            p.knownPreservedAsKnownPreserved();
        }
        catch (RemoteException ex)
        {
            test(ex.ConvertToUnhandled);
            ex.ConvertToUnhandled = false;
            throw;
        }
        test(false);
        return new ValueTask(Task.CompletedTask);
    }

    public ValueTask unknownPreservedAsBaseAsync(Ice.Current current)
    {
        SPreserved2 ex = new SPreserved2();
        ex.b = "base";
        ex.kp = "preserved";
        ex.kpd = "derived";
        ex.p1 = new SPreservedClass("bc", "spc");
        ex.p2 = ex.p1;
        throw ex;
    }

    public ValueTask
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

    public ValueTask
    relayUnknownPreservedAsBaseAsync(IRelayPrx? r, Ice.Current current)
    {
        Debug.Assert(r != null);
        IRelayPrx p = current.Connection.CreateProxy(r.Identity, IRelayPrx.Factory);
        try
        {
            p.unknownPreservedAsBase();
        }
        catch (RemoteException ex)
        {
            test(ex.ConvertToUnhandled);
            ex.ConvertToUnhandled = false;
            throw;
        }
        test(false);
        return new ValueTask(Task.CompletedTask);
    }

    public ValueTask
    relayUnknownPreservedAsKnownPreservedAsync(IRelayPrx? r, Ice.Current current)
    {
        Debug.Assert(r != null);
        IRelayPrx p = current.Connection.CreateProxy(r.Identity, IRelayPrx.Factory);
        try
        {
            p.unknownPreservedAsKnownPreserved();
        }
        catch (RemoteException ex)
        {
            test(ex.ConvertToUnhandled);
            ex.ConvertToUnhandled = false;
            throw;
        }
        test(false);
        return new ValueTask(Task.CompletedTask);
    }

    public ValueTask relayClientPrivateExceptionAsync(IRelayPrx? r, Ice.Current current)
    {
        Debug.Assert(r != null);
        IRelayPrx p = current.Connection.CreateProxy(r.Identity, IRelayPrx.Factory);
        try
        {
            p.clientPrivateException();
        }
        catch (RemoteException ex)
        {
            test(ex.ConvertToUnhandled);
            ex.ConvertToUnhandled = false;
            throw;
        }
        test(false);
        return new ValueTask(Task.CompletedTask);
    }
}
