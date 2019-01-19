//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

#include <Test.ice>

module Test
{

class SBSUnknownDerived extends SBase
{
    string sbsud;
}

class SUnknown
{
    string su;
    SUnknown cycle;
}

class D2 extends B
{
    string sd2;
    B pd2;
}

class D4 extends B
{
    B p1;
    B p2;
}

exception UnknownDerivedException extends BaseException
{
    string sude;
    D2 pd2;
}

class MyClass
{
    int i;
}

class PSUnknown extends Preserved
{
    string psu;
    PNode graph;
    MyClass cl;
}

class PSUnknown2 extends Preserved
{
    PBase pb;
}

exception PSUnknownException extends PreservedException
{
    PSUnknown2 p;
}

}
