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

class C1 { }
interface Intf { C1* op(); } // Deprecated

class C2
{
    C1* proxy; // Deprecated
}

struct S
{
    C1* proxy; // Deprecated
}

exception E
{
    C1* proxy; // Deprecated
}

sequence<C1*> Seq; // Deprecated
dictionary<string, C1*> Dict; // Deprecated

}
