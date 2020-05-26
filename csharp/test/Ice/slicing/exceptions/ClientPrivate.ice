//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[[3.7]]

#include <Test.ice>

module Test
{

class PreservedClass : BaseClass
{
    string pc;
}

exception Preserved1 : KnownPreservedDerived
{
    BaseClass p1;
}

exception Preserved2 : Preserved1
{
    BaseClass p2;
}

exception ClientPrivateException
{
    string cpe;
}

}
