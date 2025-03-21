// Copyright (c) ZeroC, Inc.

#pragma once

[["java:package:test.Ice.slicing.exceptions.client"]]
module Test
{
    //
    // Duplicate types from Test.ice. We cannot use #include since
    // that will use the types from the same prefix.
    //

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

    exception KnownPreserved extends Base
    {
        string kp;
    }

    exception KnownPreservedDerived extends KnownPreserved
    {
        string kpd;
    }

    ["format:sliced"]
    interface TestIntf
    {
        void baseAsBase() throws Base;
        void unknownDerivedAsBase() throws Base;
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
