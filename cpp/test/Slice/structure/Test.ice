// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

[["cpp:include:list"]]

module Test
{

sequence<string> StringSeq;
["cpp:type:std::list< ::Ice::Int>"] sequence<int> IntList;
dictionary<string, string> StringDict;

class C
{
    int i;
}

["cpp:class"]
struct S1
{
    string name;
}

["cpp:comparable"]
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
