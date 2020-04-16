//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice;
using Test;

public sealed class TestIntf : ITestIntf
{
    public void shutdown(Current current) => current.Adapter.Communicator.Shutdown();

    public void baseAsBase(Current current) => throw new Base("Base.b");

    public void unknownDerivedAsBase(Current current) =>
        throw new UnknownDerived("UnknownDerived.b", "UnknownDerived.ud");

    public void knownDerivedAsBase(Current current) => throw new KnownDerived("KnownDerived.b", "KnownDerived.kd");

    public void knownDerivedAsKnownDerived(Current current) =>
        throw new KnownDerived("KnownDerived.b", "KnownDerived.kd");

    public void unknownIntermediateAsBase(Current current) =>
        throw new UnknownIntermediate("UnknownIntermediate.b", "UnknownIntermediate.ui");

    public void knownIntermediateAsBase(Current current) =>
        throw new KnownIntermediate("KnownIntermediate.b", "KnownIntermediate.ki");

    public void knownMostDerivedAsBase(Current current) =>
        throw new KnownMostDerived("KnownMostDerived.b", "KnownMostDerived.ki", "KnownMostDerived.kmd");

    public void knownIntermediateAsKnownIntermediate(Current current) =>
        throw new KnownIntermediate("KnownIntermediate.b", "KnownIntermediate.ki");

    public void knownMostDerivedAsKnownIntermediate(Current current) =>
        throw new KnownMostDerived("KnownMostDerived.b", "KnownMostDerived.ki", "KnownMostDerived.kmd");

    public void knownMostDerivedAsKnownMostDerived(Current current) =>
        throw new KnownMostDerived("KnownMostDerived.b", "KnownMostDerived.ki", "KnownMostDerived.kmd");

    public void unknownMostDerived1AsBase(Current current) =>
        throw new UnknownMostDerived1("UnknownMostDerived1.b", "UnknownMostDerived1.ki", "UnknownMostDerived1.umd1");

    public void unknownMostDerived1AsKnownIntermediate(Current current) =>
        throw new UnknownMostDerived1("UnknownMostDerived1.b", "UnknownMostDerived1.ki", "UnknownMostDerived1.umd1");

    public void unknownMostDerived2AsBase(Current current) =>
        throw new UnknownMostDerived2("UnknownMostDerived2.b", "UnknownMostDerived2.ui", "UnknownMostDerived2.umd2");

    public void unknownMostDerived2AsBaseCompact(Current current) =>
        throw new UnknownMostDerived2("UnknownMostDerived2.b", "UnknownMostDerived2.ui", "UnknownMostDerived2.umd2");

    public void knownPreservedAsBase(Current current) =>
        throw new KnownPreservedDerived("base", "preserved", "derived");

    public void knownPreservedAsKnownPreserved(Current current) =>
        throw new KnownPreservedDerived("base", "preserved", "derived");

    public void serverPrivateException(Current current) => throw new ServerPrivateException("ServerPrivate");

    public void relayKnownPreservedAsBase(IRelayPrx? r, Current current)
    {
        TestHelper.Assert(r != null);
        IRelayPrx p = current.Connection!.CreateProxy(r.Identity, IRelayPrx.Factory);
        try
        {
            p.knownPreservedAsBase();
        }
        catch (RemoteException ex)
        {
            TestHelper.Assert(ex.ConvertToUnhandled);
            ex.ConvertToUnhandled = false;
            throw;
        }
        TestHelper.Assert(false);
    }

    public void relayKnownPreservedAsKnownPreserved(IRelayPrx? r, Current current)
    {
        TestHelper.Assert(r != null);
        IRelayPrx p = current.Connection!.CreateProxy(r.Identity, IRelayPrx.Factory);
        try
        {
            p.knownPreservedAsKnownPreserved();
        }
        catch (RemoteException ex)
        {
            TestHelper.Assert(ex.ConvertToUnhandled);
            ex.ConvertToUnhandled = false;
            throw;
        }
        TestHelper.Assert(false);
    }

    public void relayClientPrivateException(IRelayPrx? r, Current current)
    {
        TestHelper.Assert(r != null);
        IRelayPrx p = current.Connection!.CreateProxy(r.Identity, IRelayPrx.Factory);
        try
        {
            p.clientPrivateException();
        }
        catch (RemoteException ex)
        {
            TestHelper.Assert(ex.ConvertToUnhandled);
            ex.ConvertToUnhandled = false;
            throw;
        }
        TestHelper.Assert(false);
    }

    public void unknownPreservedAsBase(Current current)
    {
        var ex = new SPreserved2();
        ex.b = "base";
        ex.kp = "preserved";
        ex.kpd = "derived";
        ex.p1 = new SPreservedClass("bc", "spc");
        ex.p2 = ex.p1;
        throw ex;
    }

    public void unknownPreservedAsKnownPreserved(Current current)
    {
        var ex = new SPreserved2();
        ex.b = "base";
        ex.kp = "preserved";
        ex.kpd = "derived";
        ex.p1 = new SPreservedClass("bc", "spc");
        ex.p2 = ex.p1;
        throw ex;
    }

    public void relayUnknownPreservedAsBase(IRelayPrx? r, Current current)
    {
        TestHelper.Assert(r != null);
        IRelayPrx p = current.Connection!.CreateProxy(r.Identity, IRelayPrx.Factory);
        try
        {
            p.unknownPreservedAsBase();
        }
        catch (RemoteException ex)
        {
            TestHelper.Assert(ex.ConvertToUnhandled);
            ex.ConvertToUnhandled = false;
            throw;
        }
        TestHelper.Assert(false);
    }

    public void relayUnknownPreservedAsKnownPreserved(IRelayPrx? r, Ice.Current current)
    {
        TestHelper.Assert(r != null);
        IRelayPrx p = current.Connection!.CreateProxy(r.Identity, IRelayPrx.Factory);
        try
        {
            p.unknownPreservedAsKnownPreserved();
        }
        catch (RemoteException ex)
        {
            TestHelper.Assert(ex.ConvertToUnhandled);
            ex.ConvertToUnhandled = false;
            throw;
        }
        TestHelper.Assert(false);
    }
}
