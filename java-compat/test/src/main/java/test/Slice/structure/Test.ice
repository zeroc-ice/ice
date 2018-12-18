// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#pragma once

[["java:package:test.Slice.structure"]]
module Test
{

sequence<string> StringSeq;
["java:type:java.util.ArrayList<Integer>"] sequence<int> IntList;
dictionary<string, string> StringDict;

class C
{
    int i;
}

struct S1
{
    string name;
}

struct S2
{
    bool bo;
    byte by;
    short sh;
    int i;
    long l;
    float f;
    double d;
    string str;
    StringSeq ss;
    IntList il;
    StringDict sd;
    S1 s;
    C cls;
    Object* prx;
}

}
