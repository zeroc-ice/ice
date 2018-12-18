// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

module LocalTest
{

class C1
{
    int i;
}

struct S1
{
    C1 c1;
}

sequence<C1> C1Seq;
sequence<S1> S1Seq;
dictionary<int, C1> C1Dict;
dictionary<int, S1> S1Dict;

sequence<C1Seq> C1SeqSeq;
sequence<S1Seq> S1SeqSeq;

struct S2
{
    S1 s1;
}

struct S3
{
    C1Seq c1seq;
}

struct S4
{
    S1Seq s1seq;
}

struct S5
{
    C1Dict c1dict;
}

struct S6
{
    S1Dict s1dict;
}

struct S7
{
    C1SeqSeq c1seqseq;
}

struct S8
{
    S1SeqSeq s1seqseq;
}

class CB1
{
    S1 s1;
}

class CB2
{
    C1Seq c1seq;
}

class CB3
{
    S1Seq s1seq;
}

class CB4
{
    C1Dict c1dict;
}

class CB5
{
    S1Dict s1dict;
}

class CB6
{
    C1SeqSeq c1seqseq;
}

class CB7
{
    S1SeqSeq s1seqseq;
}

class CB8
{
    S1 s1;
    C1Seq c1seq;
    S1Dict s1dict;
}

struct StructKey
{
    int i;
    int j;
}

dictionary<StructKey, C1> StructDict1;
dictionary<StructKey, S1> StructDict2;

dictionary<int, C1Dict> C1DictDict;
dictionary<int, S1Dict> S1DictDict;

class Opt
{
    optional(1) S1 s1;
    optional(2) C1Seq c1seq;
    optional(3) S1Dict s1dict;
}

}
