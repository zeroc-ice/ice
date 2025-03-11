// Copyright (c) ZeroC, Inc.

#pragma once

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

    class BaseClass
    {
        string bc;
    }

    interface Relay
    {
        void knownPreservedAsBase() throws Base;
        void knownPreservedAsKnownPreserved() throws KnownPreserved;

        void unknownPreservedAsBase() throws Base;
        void unknownPreservedAsKnownPreserved() throws KnownPreserved;
    }

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

        ["format:compact"] void unknownMostDerived2AsBaseCompact() throws Base;

        void knownPreservedAsBase() throws Base;
        void knownPreservedAsKnownPreserved() throws KnownPreserved;

        void relayKnownPreservedAsBase(Relay* r) throws Base;
        void relayKnownPreservedAsKnownPreserved(Relay* r) throws KnownPreserved;

        void unknownPreservedAsBase() throws Base;
        void unknownPreservedAsKnownPreserved() throws KnownPreserved;

        void relayUnknownPreservedAsBase(Relay* r) throws Base;
        void relayUnknownPreservedAsKnownPreserved(Relay* r) throws KnownPreserved;

        void shutdown();
    }

    //
    // Types private to the client.
    //

    class PreservedClass extends BaseClass
    {
        string pc;
    }

    exception Preserved1 extends KnownPreservedDerived
    {
        BaseClass p1;
    }

    exception Preserved2 extends Preserved1
    {
        BaseClass p2;
    }
}
