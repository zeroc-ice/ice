// Copyright (c) ZeroC, Inc.

#pragma once

module Test
{
    //
    // Duplicate types from Test.ice. We cannot use #include since
    // that will use the types from the same prefix.
    //
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    exception Base
    {
        string b;
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    exception KnownDerived extends Base
    {
        string kd;
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    exception KnownIntermediate extends Base
    {
        string ki;
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    exception KnownMostDerived extends KnownIntermediate
    {
        string kmd;
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    exception KnownPreserved extends Base
    {
        string kp;
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    exception KnownPreservedDerived extends KnownPreserved
    {
        string kpd;
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    class BaseClass
    {
        string bc;
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    interface Relay
    {
        void knownPreservedAsBase() throws Base;
        void knownPreservedAsKnownPreserved() throws KnownPreserved;
<<<<<<< Updated upstream

        void unknownPreservedAsBase() throws Base;
        void unknownPreservedAsKnownPreserved() throws KnownPreserved;
    }

=======

        void unknownPreservedAsBase() throws Base;
        void unknownPreservedAsKnownPreserved() throws KnownPreserved;
    }

>>>>>>> Stashed changes
    interface TestIntf
    {
        void baseAsBase() throws Base;
        void unknownDerivedAsBase() throws Base;
        void knownDerivedAsBase() throws Base;
        void knownDerivedAsKnownDerived() throws KnownDerived;
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
        void unknownIntermediateAsBase() throws Base;
        void knownIntermediateAsBase() throws Base;
        void knownMostDerivedAsBase() throws Base;
        void knownIntermediateAsKnownIntermediate() throws KnownIntermediate;
        void knownMostDerivedAsKnownIntermediate() throws KnownIntermediate;
        void knownMostDerivedAsKnownMostDerived() throws KnownMostDerived;
<<<<<<< Updated upstream

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

=======

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

>>>>>>> Stashed changes
    class PreservedClass extends BaseClass
    {
        string pc;
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    exception Preserved1 extends KnownPreservedDerived
    {
        BaseClass p1;
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    exception Preserved2 extends Preserved1
    {
        BaseClass p2;
    }
}
