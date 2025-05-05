// Copyright (c) ZeroC, Inc.

#pragma once

["matlab:identifier:classdef_"]
module classdef
{
    ["matlab:identifier:persistent_"]
    enum persistent
    {
        ["matlab:identifier:break_"] break,
        ["matlab:identifier:catch_"] catch,
        LAST
    }

    ["matlab:identifier:global_"]
    struct global
    {
        ["matlab:identifier:case_"] persistent case = catch;
        ["matlab:identifier:continue_"] int continue = 1;
        ["matlab:identifier:eq_"] int eq = 2;
    }

    ["matlab:identifier:logical_"]
    class logical
    {
        ["matlab:identifier:else_"] persistent else = break;
        ["matlab:identifier:for_"] global for;
        bool int64 = true;
    }

    ["matlab:identifier:escaped_xor"]
    class xor extends logical
    {
        ["matlab:identifier:return_"] int return = 1;
    }

    ["matlab:identifier:parfor_"] sequence<global> parfor;
    ["matlab:identifier:switch_"] dictionary<int, global> switch;

    class Base
    {
        ["matlab:identifier:while_"] int while = 1;
    }

    class Derived extends Base
    {
        ["matlab:identifier:if_"] int if = 2;
        ["matlab:identifier:spmd_"] parfor spmd;
        ["matlab:identifier:otherwise_"] switch otherwise;
    }

    ["matlab:identifier:bitand_"]
    exception bitand
    {
        // These data members clash with members of MException.
        ["matlab:identifier:identifier_"] string identifier = "1";
        ["matlab:identifier:message_"] string message = "2";

        ["matlab:identifier:end_"] logical end;
    }

    ["matlab:identifier:escaped_bitor"]
    exception bitor extends bitand
    {
        ["matlab:identifier:enumeration_"]
        int enumeration = 1;
    }

    ["matlab:identifier:MyInterface"]
    interface operations
    {
        ["matlab:identifier:foobar"] void events();
        ["matlab:identifier:func"] void function();
    }

    ["matlab:identifier:methods_"]
    const int methods = 1;
}
