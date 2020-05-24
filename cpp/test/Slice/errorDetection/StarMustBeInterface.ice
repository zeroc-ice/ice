//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

module Test
{

class C1 { }
interface Intf { C1* op(); } // Must be an interface

class C2
{
    C1* proxy; // Must be an interface
}

struct S
{
    C1* proxy; // Must be an interface
}

exception E
{
    C1* proxy; // Must be an interface
}

sequence<C1*> Seq; // Must be an interface
dictionary<string, C1*> Dict; // Must be an interface

}
