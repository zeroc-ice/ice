// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

[["cpp:include:list"]]

module Test
{

sequence<int> IntSeq;
dictionary<string, string> StringDict;

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
    string str;
    IntSeq seq;
    S1 s;
}

class C
{
    string name;
}

struct S3
{
    C obj;
    StringDict sd;
    Object* prx;
}

}
