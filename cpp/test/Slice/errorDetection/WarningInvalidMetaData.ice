//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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

    ["cpp:type:std::list<::std::string>"]
    void op1();

    ["cpp:array"]
    void op2();

    void op3(["cpp:type:my_string"] string s);

    void op4(["cpp:view-type:my_string"] string s);
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

["bad", "cpp:bad", "java:bad"] // We only validate metadata when it has an applicable language prefix.
class C
{
}

["cpp98:foo", "cpp11:bar"] // The cpp98 and cpp11 attributes were removed in 3.8. We issue a friendly warning.
class P
{
}

}
