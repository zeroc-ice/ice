// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#pragma once

#include <Test.ice>

module Test
{

class PreservedClass extends BaseClass
{
    string pc;
}

exception Preserved1 extends KnownPreservedDerived
{
    BaseClass p1;
}

exception Preserved2 extends Preserved1
{
    BaseClass p2;
}

}
