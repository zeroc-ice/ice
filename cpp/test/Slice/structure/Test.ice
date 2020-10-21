//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

[[cpp:include(list)]]

module Test
{

sequence<string> StringSeq;
[cpp:type(std::list< ::Ice::Int>)] sequence<int> IntList;
dictionary<string, string> StringDict;

class C
{
    int i;
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
    C cls;
    Object* prx;
}

}
