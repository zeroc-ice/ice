//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[[3.7]]
[[suppress-warning(reserved-identifier)]]

module ZeroC::Ice::Test::Slicing::Objects
{

class SBase
{
    string sb = "";
}

class SBSKnownDerived : SBase
{
    string sbskd = "";
}

class B
{
    string sb = "";
    B pb;
}

class D1 : B
{
    string sd1 = "";
    B pd1;
}

sequence<B> BSeq;

class SS1
{
    BSeq s;
}

class SS2
{
    BSeq s;
}

struct SS3
{
    SS1 c1;
    SS2 c2;
}

dictionary<int, B> BDict;

exception BaseException
{
    string sbe;
    B pb;
}

exception DerivedException : BaseException
{
    string sde;
    D1 pd1;
}

class Forward;

class PBase
{
    int pi;
}

sequence<PBase> PBaseSeq;

[preserve-slice]
class Preserved : PBase
{
    string ps = "";
}

class PDerived : Preserved
{
    PBase pb;
}

class CompactPDerived(56) : Preserved
{
    PBase pb;
}

[preserve-slice]
class PNode
{
    PNode next;
}

exception PreservedException
{
}

[format(sliced)]
interface TestIntf
{
    Object SBaseAsObject();
    SBase SBaseAsSBase();
    SBase SBSKnownDerivedAsSBase();
    SBSKnownDerived SBSKnownDerivedAsSBSKnownDerived();

    SBase SBSUnknownDerivedAsSBase();
    void CUnknownAsSBase(SBase cUnknown);

    [format(compact)] SBase SBSUnknownDerivedAsSBaseCompact();

    Object SUnknownAsObject();
    void checkSUnknown(Object o);

    B oneElementCycle();
    B twoElementCycle();
    B D1AsB();
    D1 D1AsD1();
    B D2AsB();

    (B r1, B r2) paramTest1();
    (B r1, B r2) paramTest2();
    (B r1, B r2, B r3) paramTest3();
    (B r1, B r2) paramTest4();

    (B r1, B r2, B r3) returnTest1();
    (B r1, B r2, B r3) returnTest2();
    B returnTest3(B p1, B p2);

    SS3 sequenceTest(SS1 p1, SS2 p2);

    (BDict r1, BDict r2) dictionaryTest(BDict bin);

    PBase exchangePBase(PBase pb);

    Preserved PBSUnknownAsPreserved();
    void checkPBSUnknown(Preserved p);

    [amd] Preserved PBSUnknownAsPreservedWithGraph();
    void checkPBSUnknownWithGraph(Preserved p);

    [amd] Preserved PBSUnknown2AsPreservedWithGraph();
    void checkPBSUnknown2WithGraph(Preserved p);

    PNode exchangePNode(PNode pn);

    void throwBaseAsBase();
    void throwDerivedAsBase();
    void throwDerivedAsDerived();
    void throwUnknownDerivedAsBase();
    [amd] void throwPreservedException();

    Forward useForward(); /* Use of forward-declared class to verify that code is generated correctly. */

    void shutdown();
}

class Hidden
{
    Forward f;
}

class Forward
{
    Hidden h;
}

}
