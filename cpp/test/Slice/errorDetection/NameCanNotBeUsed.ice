// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************



module Test
{

class Foo
{
    void Foo();
    long l;
}

interface IFoo
{
    void IFoo();
}

class Bar
{
    string Bar; // Valid as Ice-3.6 (member names has its own scope)
    long l;
}

exception EBar
{
    string EBar; // Valid as Ice-3.6 (member names has its own scope)
}

struct SBar
{
    string SBar; // Valid as Ice-3.6 (member names has its own scope)
}

}
