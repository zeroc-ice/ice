//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[[3.7]]
[[suppress-warning(reserved-identifier)]]

#include <Test.ice>

module ZeroC::Ice::Test::Slicing::Objects
{

class SBSUnknownDerived : SBase
{
    string sbsud;
}

class SUnknown
{
    string su;
    SUnknown cycle;
}

class D2 : B
{
    string sd2 = "";
    B pd2;
}

class D4 : B
{
    B p1;
    B p2;
}

exception UnknownDerivedException : BaseException
{
    string sude;
    D2 pd2;
}

class MyClass
{
    int i;
}

class PSUnknown : Preserved
{
    string psu;
    PNode graph;
    MyClass cl;
}

class PSUnknown2 : Preserved
{
    PBase pb;
}

exception PSUnknownException : PreservedException
{
    PSUnknown2 p;
}

}
