// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef TEST_ICE
#define TEST_ICE

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

//
// Remainder of definitions are there to test that the generated code compiles;
// they are not used for the actual run-time tests.
//

// Sequence of classes defined above.

// Sequence of structs defined above.


sequence<C2Dict> C2DictSeq;		// Sequence of dictionary.

sequence<CSeq> CSeqSeq;			// Sequence of sequence.

// Struct containing classes defined above.

struct A				// Struct containing sequence.
{
    SSeq theSSeq;
};

// Struct containing dictionary defined above.

struct B				// Struct containing struct.
{
    S theS;
};

// Dictionary of classes defined above.

dictionary<int, CSeq> CSeqDict;		// Dictionary containing sequence.

dictionary<int, S> SDict;		// Dictionary containing struct.

dictionary<int, CDict> CDictDict;	// Dictionary containing dictionary.

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

#endif
