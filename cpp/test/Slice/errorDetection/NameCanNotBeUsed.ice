// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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
};

interface IFoo
{
    void IFoo();
};

class Bar
{
    string Bar;
    long l;
};

exception EBar
{
    string EBar;
};

struct SBar
{
    string SBar;
};

};
