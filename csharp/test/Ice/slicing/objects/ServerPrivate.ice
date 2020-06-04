//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[[3.7]]

#include <Test.ice>

// 'Test.ice' and 'TestAMD.ice' need to generate code into separate namespaces, but with identical type-ids. So we use
// the 'cs:namespace' metadata here to place the AMD code into a separate AMD namespace, instead of another module;
// Which would result in differing type-ids.
[cs:namespace:ZeroC.Ice.Test]
module Slicing::Objects
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
