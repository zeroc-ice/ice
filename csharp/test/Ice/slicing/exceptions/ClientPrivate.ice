// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
