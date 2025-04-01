// Copyright (c) ZeroC, Inc.

using Test;

public sealed class TestI : TestIntfDisp_
{
    public override Task shutdownAsync(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
        return Task.CompletedTask;
    }

    public override Task baseAsBaseAsync(Ice.Current current) => throw new Base("Base.b");

    public override Task unknownDerivedAsBaseAsync(Ice.Current current) =>
        throw new UnknownDerived("UnknownDerived.b", "UnknownDerived.ud");

    public override Task knownDerivedAsBaseAsync(Ice.Current current) =>
        throw new KnownDerived("KnownDerived.b", "KnownDerived.kd");

    public override Task
    knownDerivedAsKnownDerivedAsync(Ice.Current current) =>
        throw new KnownDerived("KnownDerived.b", "KnownDerived.kd");

    public override Task
    unknownIntermediateAsBaseAsync(Ice.Current current) =>
        throw new UnknownIntermediate("UnknownIntermediate.b", "UnknownIntermediate.ui");

    public override Task
    knownIntermediateAsBaseAsync(Ice.Current current) =>
        throw new KnownIntermediate("KnownIntermediate.b", "KnownIntermediate.ki");

    public override Task
    knownMostDerivedAsBaseAsync(Ice.Current current) =>
        throw new KnownMostDerived("KnownMostDerived.b", "KnownMostDerived.ki", "KnownMostDerived.kmd");

    public override Task
    knownIntermediateAsKnownIntermediateAsync(Ice.Current current) =>
        throw new KnownIntermediate("KnownIntermediate.b", "KnownIntermediate.ki");

    public override Task
    knownMostDerivedAsKnownIntermediateAsync(Ice.Current current) =>
        throw new KnownMostDerived("KnownMostDerived.b", "KnownMostDerived.ki", "KnownMostDerived.kmd");

    public override Task
    knownMostDerivedAsKnownMostDerivedAsync(Ice.Current current) =>
        throw new KnownMostDerived("KnownMostDerived.b", "KnownMostDerived.ki", "KnownMostDerived.kmd");

    public override Task
    unknownMostDerived1AsBaseAsync(Ice.Current current) =>
        throw new UnknownMostDerived1("UnknownMostDerived1.b", "UnknownMostDerived1.ki", "UnknownMostDerived1.umd1");

    public override Task
    unknownMostDerived1AsKnownIntermediateAsync(Ice.Current current) =>
        throw new UnknownMostDerived1("UnknownMostDerived1.b", "UnknownMostDerived1.ki", "UnknownMostDerived1.umd1");

    public override Task
    unknownMostDerived2AsBaseAsync(Ice.Current current) =>
        throw new UnknownMostDerived2("UnknownMostDerived2.b", "UnknownMostDerived2.ui", "UnknownMostDerived2.umd2");
}
