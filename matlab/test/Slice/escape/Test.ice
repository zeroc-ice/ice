// Copyright (c) ZeroC, Inc.

#pragma once

// TODO come up with a better way to escape modules.
module escaped_classdef
{
    enum bitand
    {
        ["matlab:identifier:break_"] break,
        ["matlab:identifier:catch_"] catch,
        LAST
    }

    struct bitor
    {
        ["matlab:identifier:case_"] bitand case = catch;
        ["matlab:identifier:continue_"] int continue = 1;
        ["matlab:identifier:eq_"] int eq = 2;
    }

    class logical
    {
        ["matlab:identifier:else_"] bitand else = break;
        ["matlab:identifier:for_"] bitor for;
        bool int64 = true;
    }

    ["matlab:identifier:xor_"]
    class xor extends logical
    {
        ["matlab:identifier:return_"] int return = 1;
    }

    ["matlab:identifier:parfor_"] sequence<bitor> parfor;
    ["matlab:identifier:switch_"] dictionary<int, bitor> switch;

    ["matlab:identifier:try_"]
    class try
    {
        ["matlab:identifier:while_"] int while = 1;
    }

    ["matlab:identifier:properties_"]
    class properties extends try
    {
        ["matlab:identifier:if_"] int if = 2;
        ["matlab:identifier:spmd_"] parfor spmd;
        ["matlab:identifier:otherwise_"] switch otherwise;
    }

    ["matlab:identifier:persistent_"]
    exception persistent
    {
        // These data members clash with members of MException.
        ["matlab:identifier:identifier_"] string identifier = "1";
        ["matlab:identifier:message_"] string message = "2";

        ["matlab:identifier:end_"] logical end;
    }

    ["matlab:identifier:global_"]
    exception global extends persistent
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
