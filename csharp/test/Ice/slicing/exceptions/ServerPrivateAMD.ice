// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#pragma once

#include <TestAMD.ice>

module Test
{

exception UnknownDerived extends Base
{
    string ud;
}

exception UnknownIntermediate extends Base
{
   string ui;
}

exception UnknownMostDerived1 extends KnownIntermediate
{
   string umd1;
}

exception UnknownMostDerived2 extends UnknownIntermediate
{
   string umd2;
}

class SPreservedClass extends BaseClass
{
    string spc;
}

exception SPreserved1 extends KnownPreservedDerived
{
    BaseClass p1;
}

exception SPreserved2 extends SPreserved1
{
    BaseClass p2;
}

}
