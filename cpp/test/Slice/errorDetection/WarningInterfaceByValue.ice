// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

module Test
{

interface Intf1 { }
interface Intf2 { Intf1 op(); } // Deprecated

class C
{
    Intf1 i; // Deprecated
}

struct S
{
    Intf1 i; // Deprecated
}

exception E
{
    Intf1 i; // Deprecated
}

local interface LIntf1 { }
local interface LIntf2
{
    LIntf1 op(); // OK
    Intf1 op2(); // Deprecated
}

sequence<Intf1> Seq; // Deprecated
dictionary<string, Intf1> Dict; // Deprecated

}
