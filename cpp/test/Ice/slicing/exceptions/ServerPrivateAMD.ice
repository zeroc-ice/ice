// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
