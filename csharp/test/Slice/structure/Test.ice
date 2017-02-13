// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

module Test
{

sequence<string> StringSeq;
["clr:collection"] sequence<int> IntList;
dictionary<string, string> StringDict;

class C
{
    int i;
};

["clr:class"]
struct S1
{
    string name;
};

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
};

};
