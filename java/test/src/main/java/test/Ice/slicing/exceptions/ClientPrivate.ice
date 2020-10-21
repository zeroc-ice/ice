//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[[java:package(test.Ice.slicing.exceptions.client)]]
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

exception KnownDerived : Base
{
    string kd;
}

exception KnownIntermediate : Base
{
    string ki;
}

exception KnownMostDerived : KnownIntermediate
{
    string kmd;
}

[preserve-slice]
exception KnownPreserved : Base
{
    string kp;
}

exception KnownPreservedDerived : KnownPreserved
{
    string kpd;
}

[preserve-slice]
class BaseClass
{
    string bc;
}

[format(sliced)]
interface Relay
{
    void knownPreservedAsBase() throws Base;
    void knownPreservedAsKnownPreserved() throws KnownPreserved;

    void unknownPreservedAsBase() throws Base;
    void unknownPreservedAsKnownPreserved() throws KnownPreserved;
}

[format(sliced)]
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

    [format(compact)] void unknownMostDerived2AsBaseCompact() throws Base;

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

class PreservedClass : BaseClass
{
    string pc;
}

exception Preserved1 : KnownPreservedDerived
{
    BaseClass p1;
}

exception Preserved2 : Preserved1
{
    BaseClass p2;
}

}
