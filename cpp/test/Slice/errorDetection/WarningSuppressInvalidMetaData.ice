// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

[["suppress-warning:invalid-metadata"]]

[["cpp:header-ext:hh"]]
[["cpp:header-ext:hh"]]

[["cpp:source-ext:cc"]]
[["cpp:source-ext:cc"]]

[["cpp:dll-export:Test"]]
[["cpp:dll-export:Test"]]

[["cpp:header-ext"]]
[["cpp:header-ext:"]]

[["cpp:source-ext"]]
[["cpp:source-ext:"]]

[["cpp:dll-export"]]
[["cpp:dll-export:"]]

[["cpp:include"]]
[["cpp:include:"]]

module Test
{

interface I
{
    ["cpp:noexcept"]
    void op();

    ["cpp:type:std::list< ::std::string>"]
    void op1();

    ["cpp:view-type:std::experimental::string_view"]
    void op2();

    ["cpp:array"]
    void op3();

    ["cpp:range"]
    void op4();
}

["cpp:class", "cpp:comparable"]
class C
{
}

["cpp:const", "cpp:ice_print"]
struct S
{
    int i;
}

["cpp:virtual"]
exception E
{
}

}
