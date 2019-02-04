//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

[["cpp:include:list"]]

module Test
{

sequence<string> StringSeq;
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
    StringDict sd;
    S1 s;
    C cls;
    Object* prx;
}

}
