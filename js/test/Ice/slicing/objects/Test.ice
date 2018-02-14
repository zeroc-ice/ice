// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

module Test
{

//
// Duplicate types from Test.ice. We cannot use #include since
// that will use the types from the same prefix.
//

class SBase
{
    string sb;
};

class SBSKnownDerived extends SBase
{
    string sbskd;
};

class B
{
    string sb;
    B pb;
};

class D1 extends B
{
    string sd1;
    B pd1;
};

sequence<B> BSeq;

class SS1
{
    BSeq s;
};

class SS2
{
    BSeq s;
};

struct SS3
{
    SS1 c1;
    SS2 c2;
};

dictionary<int, B> BDict;

exception BaseException
{
    string sbe;
    B pb;
};

exception DerivedException extends BaseException
{
    string sde;
    D1 pd1;
};

class Forward;          /* Forward-declared class defined in another compilation unit */

class PBase
{
    int pi;
};

sequence<PBase> PBaseSeq;

["preserve-slice"]
class Preserved extends PBase
{
    string ps;
};

class PDerived extends Preserved
{
    PBase pb;
};

class CompactPDerived(56) extends Preserved
{
    PBase pb;
};

["preserve-slice"]
class PNode
{
    PNode next;
};

["preserve-slice"]
exception PreservedException
{
};

["format:sliced"]
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

    ["amd"] Preserved PBSUnknownAsPreservedWithGraph();
    void checkPBSUnknownWithGraph(Preserved p);

    ["amd"] Preserved PBSUnknown2AsPreservedWithGraph();
    void checkPBSUnknown2WithGraph(Preserved p);

    PNode exchangePNode(PNode pn);

    void throwBaseAsBase() throws BaseException;
    void throwDerivedAsBase() throws BaseException;
    void throwDerivedAsDerived() throws DerivedException;
    void throwUnknownDerivedAsBase() throws BaseException;
    ["amd"] void throwPreservedException() throws PreservedException;

    void useForward(out Forward f); /* Use of forward-declared class to verify that code is generated correctly. */

    void shutdown();
};

//
// Types private to the client.
//

class D3 extends B
{
    string sd3;
    B pd3;
};

["preserve-slice"]
class PCUnknown extends PBase
{
    string pu;
};

class PCDerived extends PDerived
{
    PBaseSeq pbs;
};

class PCDerived2 extends PCDerived
{
    int pcd2;
};

class PCDerived3 extends PCDerived2
{
    Object pcd3;
};

class CompactPCDerived(57) extends CompactPDerived
{
    PBaseSeq pbs;
};

class Hidden
{
    Forward f;
};

class Forward
{
    Hidden h;
};

};
