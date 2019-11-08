//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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

sequence<Intf1> Seq; // Deprecated
dictionary<string, Intf1> Dict; // Deprecated

}
