// Copyright (c) ZeroC, Inc.
#pragma once

[["swift:class-resolver-prefix:IceSlicingExceptions"]]
module Test
{
    exception Base
    {
        string b;
    }

    exception KnownDerived extends Base
    {
        string kd;
    }

    exception KnownIntermediate extends Base
    {
        string ki;
    }

    exception KnownMostDerived extends KnownIntermediate
    {
        string kmd;
    }

    interface TestIntf
    {
        void baseAsBase() throws Base;

        // Test that the compact metadata is ignored (exceptions are always encoded with the sliced format).
        ["format:compact"] void unknownDerivedAsBase() throws Base;

        void knownDerivedAsBase() throws Base;
        void knownDerivedAsKnownDerived() throws KnownDerived;

        void unknownIntermediateAsBase() throws Base;
        void knownIntermediateAsBase() throws Base;
        void knownMostDerivedAsBase() throws Base;
        void knownIntermediateAsKnownIntermediate() throws KnownIntermediate;
        void knownMostDerivedAsKnownIntermediate() throws KnownIntermediate;
        void knownMostDerivedAsKnownMostDerived() throws KnownMostDerived;

        void unknownMostDerived1AsBase() throws Base;
        void unknownMostDerived1AsKnownIntermediate() throws KnownIntermediate;
        void unknownMostDerived2AsBase() throws Base;

        void shutdown();
    }
}
