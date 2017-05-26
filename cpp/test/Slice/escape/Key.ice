// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

[["underscore"]]
[["suppress-warning:deprecated"]]

module and
{

enum continue
{
    asm
}

struct auto
{
    int default;
}

["cpp:class"] struct delete
{
    string else;
}

interface break
{
    ["amd"] void case(int catch, out int try);
}

interface char
{
    void explicit();
}

class switch
{
    int if;
    char* export;
    int volatile;
}

interface do extends char, break
{
}

sequence<auto> extern;

dictionary<string,auto> for;

exception return
{
    int signed;
}

exception sizeof extends return
{
    int static; int switch;
}

local interface friend
{
    auto goto(continue if, auto d, delete inline, switch private, do mutable, break* namespace,
              char* new, switch* not, do* operator, int or, int protected, int public, int register)
        throws return, sizeof;
}

const int template = 0;
const int this = 0;
const int throw = 0;
const int typedef = 0;
const int typeid = 0;
const int typename = 0;
const int union = 0;
const int unsigned = 0;
const int using = 0;
const int virtual = 0;
const int while = 0;
const int xor = 0;
const int xor_eq = 0;

}
