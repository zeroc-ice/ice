// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

module Test
{

class C
{
    C left;
    C right;
};
sequence<C> CSeq;

dictionary<int, C> CDict;

struct S
{
    C theC;
};
sequence<S> SSeq;

class C2;
dictionary<int, C2> C2Dict;

struct S2
{
    C2Dict theC2Dict;
};

sequence<S2> S2Seq;

class C2
{
    S2Seq theS2Seq;
};

class Leaf
{
    int i;
};

class Node
{
    Leaf l;
    Node n;
};

//
// Remainder of definitions are there to test that the generated code compiles;
// they are not used for the actual run-time tests.
//

// Sequence of classes defined above.

// Sequence of structs defined above.


sequence<C2Dict> C2DictSeq;             // Sequence of dictionary.

sequence<CSeq> CSeqSeq;                 // Sequence of sequence.

// Struct containing classes defined above.

struct A                                // Struct containing sequence.
{
    SSeq theSSeq;
};

// Struct containing dictionary defined above.

struct B                                // Struct containing struct.
{
    S theS;
};

// Dictionary of classes defined above.

dictionary<int, CSeq> CSeqDict;         // Dictionary containing sequence.

dictionary<int, S> SDict;               // Dictionary containing struct.

dictionary<int, CDict> CDictDict;       // Dictionary containing dictionary.

class CTest
{
    CSeq theCSeq;
    SSeq theSSeq;
    C2DictSeq theC2DictSeq;
    CSeqSeq theCSeqSeq;
    S theS;
    A theA;
    S2 theS2;
    B theB;
    CDict theCDict;
    CSeqDict theCSeqDict;
    SDict theSDict;
    CDictDict theCDictDict;
};

module AAA
{
    class B;

    sequence<B> BSeq;
    dictionary<int, B> BDict;
    struct BStruct
    {
        B bMem;
    };

    class C
    {
        Object obj1;
        Object obj2;
        B b1;
        B b2;
        BSeq x;
        BDict y;
        BStruct z;
    };
};

module AAA
{
    sequence<C> CSeq;
    dictionary<int, C> CDict;
    struct CStruct
    {
        C cMem;
    };

    class B
    {
        Object obj1;
        Object obj2;
        C c1;
        C c2;
        CSeq x;
        CDict y;
        CStruct z;
    };
};

module CCC
{
    class Forward;
    sequence<Forward*> ForwardProxySeq;
};

module CCC
{
    class Forward
    {
    };
};

module DDD
{     
    interface I
    {
        void op();
    };

    class C
    {
    };

    class C2;

    class U
    { 
        I myI;
        I* myIstar;
        C myC;
        C* myCstar;
        C2 myC2;
        C2* myC2star;
    };

    class C2
    {
    };
};

};
