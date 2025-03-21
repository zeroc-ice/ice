// Copyright (c) ZeroC, Inc.

#pragma once

[["java:package:test.Ice.slicing.exceptions.serverAMD"]]
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

    ["amd"]
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

    //
    // Types private to the server.
    //

    exception UnknownDerived extends Base
    {
        string ud;
    }

    exception UnknownIntermediate extends Base
    {
        string ui;
    }

    exception UnknownMostDerived1 extends KnownIntermediate
    {
        string umd1;
    }

    exception UnknownMostDerived2 extends UnknownIntermediate
    {
        string umd2;
    }
}
