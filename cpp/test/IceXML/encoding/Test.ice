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

module Test
{

struct Struct1
{
    long l;
};

struct Struct2
{
    Struct1 s1;
};

struct Struct3
{
    long l;
    Struct2 s2;
};

struct Struct4
{
    Struct3 s3;
    long l;
};

sequence<Struct4> Struct4Seq;

dictionary<string, Struct4> StringStruct4Dict;

enum Color
{
    Red,
    Green,
    Blue
};

sequence<Color> ColorSeq;

class Class1
{
    Color c;
    string name;
};

class Class2 extends Class1
{
    Class2 r;
};

interface Interface1
{
    void if1();
};

class Class3 extends Class2 implements Interface1
{
};

exception Exception1
{
};

exception Exception2 extends Exception1
{
    string msg;
};

};

#endif
