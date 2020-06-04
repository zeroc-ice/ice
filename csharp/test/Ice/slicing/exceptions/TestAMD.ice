//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[[3.7]]

// 'Test.ice' and 'TestAMD.ice' need to generate code into separate namespaces, but with identical type-ids. So we use
// the 'cs:namespace' metadata here to place the AMD code into a separate AMD namespace, instead of another module;
// Which would result in differing type-ids.
[cs:namespace:ZeroC.Ice.Test.AMD]
module Slicing::Exceptions
{

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

[format:sliced]
interface Relay
{
    void knownPreservedAsBase() throws Base;
    void knownPreservedAsKnownPreserved() throws KnownPreserved;

    void unknownPreservedAsBase() throws Base;
    void unknownPreservedAsKnownPreserved() throws KnownPreserved;

    void clientPrivateException();
}

[amd] [format:sliced]
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

    [format:compact] void unknownMostDerived2AsBaseCompact() throws Base;

    void knownPreservedAsBase() throws Base;
    void knownPreservedAsKnownPreserved() throws KnownPreserved;

    void serverPrivateException();

    void relayKnownPreservedAsBase(Relay* r) throws Base;
    void relayKnownPreservedAsKnownPreserved(Relay* r) throws KnownPreserved;

    void unknownPreservedAsBase() throws Base;
    void unknownPreservedAsKnownPreserved() throws KnownPreserved;

    void relayUnknownPreservedAsBase(Relay* r) throws Base;
    void relayUnknownPreservedAsKnownPreserved(Relay* r) throws KnownPreserved;

    void relayClientPrivateException(Relay* r);

    void shutdown();
}

}
