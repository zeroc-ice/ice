// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["java:package:test.Ice.slicing.objects.serverAMD"]]
module Test
{

//
// Duplicate types from Test.ice. We cannot use #include since
// that will use the types from the same prefix.
//

class SBase
{
    string sb;
}

class SBSKnownDerived extends SBase
{
    string sbskd;
}

class B
{
    string sb;
    B pb;
}

class D1 extends B
{
    string sd1;
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

exception DerivedException extends BaseException
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

["preserve-slice"]
class Preserved extends PBase
{
    string ps;
}

class PDerived extends Preserved
{
    PBase pb;
}

["preserve-slice"]
class PNode
{
    PNode next;
}

["preserve-slice"]
exception PreservedException
{
}

["amd", "format:sliced"]
interface TestIntf
{
    Object SBaseAsObject();
    SBase SBaseAsSBase();
    SBase SBSKnownDerivedAsSBase();
    SBSKnownDerived SBSKnownDerivedAsSBSKnownDerived();

    SBase SBSUnknownDerivedAsSBase();

    ["format:compact"] SBase SBSUnknownDerivedAsSBaseCompact();

    Object SUnknownAsObject();
    void checkSUnknown(Object o);

    B oneElementCycle();
    B twoElementCycle();
    B D1AsB();
    D1 D1AsD1();
    B D2AsB();

    void paramTest1(out B p1, out B p2);
    void paramTest2(out B p2, out B p1);
    B paramTest3(out B p1, out B p2);
    B paramTest4(out B p);

    B returnTest1(out B p1, out B p2);
    B returnTest2(out B p2, out B p1);
    B returnTest3(B p1, B p2);

    SS3 sequenceTest(SS1 p1, SS2 p2);

    BDict dictionaryTest(BDict bin, out BDict bout);

    PBase exchangePBase(PBase pb);

    Preserved PBSUnknownAsPreserved();
    void checkPBSUnknown(Preserved p);

    Preserved PBSUnknownAsPreservedWithGraph();
    void checkPBSUnknownWithGraph(Preserved p);

    Preserved PBSUnknown2AsPreservedWithGraph();
    void checkPBSUnknown2WithGraph(Preserved p);

    PNode exchangePNode(PNode pn);

    void throwBaseAsBase() throws BaseException;
    void throwDerivedAsBase() throws BaseException;
    void throwDerivedAsDerived() throws DerivedException;
    void throwUnknownDerivedAsBase() throws BaseException;
    void throwPreservedException() throws PreservedException;

    void useForward(out Forward f); /* Use of forward-declared class to verify that code is generated correctly. */

    void shutdown();
}

//
// Types private to the server.
//

class SBSUnknownDerived extends SBase
{
    string sbsud;
}

class SUnknown
{
    string su;
    SUnknown cycle;
}

class D2 extends B
{
    string sd2;
    B pd2;
}

class D4 extends B
{
    B p1;
    B p2;
}

exception UnknownDerivedException extends BaseException
{
    string sude;
    D2 pd2;
}

class MyClass
{
    int i;
}

class PSUnknown extends Preserved
{
    string psu;
    PNode graph;
    MyClass cl;
}

class PSUnknown2 extends Preserved
{
    PBase pb;
}

exception PSUnknownException extends PreservedException
{
    PSUnknown2 p;
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
