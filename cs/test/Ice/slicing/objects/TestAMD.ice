// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_AMD_ICE
#define TEST_AMD_ICE

module Test
{

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

["clr:collection"] sequence<B> BSeq;

class SS1
{
    BSeq s;
};

class SS2
{
    BSeq s;
};

struct SS
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

class Forward;          // Forward-declared class defined in another compilation unit

["ami", "amd"] interface TestIntf
{
    Object SBaseAsObject();
    SBase SBaseAsSBase();
    SBase SBSKnownDerivedAsSBase();
    SBSKnownDerived SBSKnownDerivedAsSBSKnownDerived();

    SBase SBSUnknownDerivedAsSBase();

    Object SUnknownAsObject();

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

    SS sequenceTest(SS1 p1, SS2 p2);

    BDict dictionaryTest(BDict bin, out BDict bout);

    void throwBaseAsBase() throws BaseException;
    void throwDerivedAsBase() throws BaseException;
    void throwDerivedAsDerived() throws DerivedException;
    void throwUnknownDerivedAsBase() throws BaseException;

    void useForward(out Forward f);     // Use of forward-declared class to verify that code is generated correctly.

    void shutdown();
};

};

#endif
