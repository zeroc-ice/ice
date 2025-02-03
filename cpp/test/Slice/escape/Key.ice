// Copyright (c) ZeroC, Inc.

// TODO figure out how to escape identifiers on modules!
//["cpp:identifier:cpp_and"] This should be `module and` which is mapped to `_cpp_and`.
module cpp_and
{
    ["cpp:identifier:cpp_continue"]
    enum continue
    {
        ["cpp:identifier:cpp_asm"]
        asm
    }

    ["cpp:identifier:cpp_auto"]
    struct auto
    {
        ["cpp:identifier:cpp_default"]
        int default;
    }

    ["cpp:identifier:cpp_break"]
    interface break
    {
        ["amd", "cpp:identifier:cpp_case"] void case(
            ["cpp:identifier:cpp_catch"] int catch,
            out ["cpp:identifier:cpp_try"] int try
        );

        ["cpp:identifier:cpp_explicit"] void explicit();
    }

    ["cpp:identifier:cpp_switch"]
    class switch
    {
        ["cpp:identifier:cpp_if"] int if;
        ["cpp:identifier:cpp_export"] break* export;
        ["cpp:identifier:cpp_volatile"] int volatile;
    }

    ["cpp:identifier:cpp_do"]
    interface do extends break
    {
    }

    ["cpp:identifier:cpp_extern"]
    sequence<auto> extern;

    ["cpp:identifier:cpp_for"]
    dictionary<string,auto> for;

    ["cpp:identifier:cpp_return"]
    exception return
    {
        ["cpp:identifier:cpp_signed"]
        int signed;
    }

    ["cpp:identifier:cpp_sizeof"]
    exception sizeof extends return
    {
        ["cpp:identifier:cpp_static"] int static;
        ["cpp:identifier:cpp_switch"] int switch;
    }

    ["cpp:identifier:cpp_friend"]
    interface friend
    {
        ["cpp:identifier:cpp_goto"]
        auto goto(
            ["cpp:identifier:cpp_if"] continue if,
            ["cpp:identifier:cpp_d"] auto d,
            ["cpp:identifier:cpp_private"] switch private,
            ["cpp:identifier:cpp_mutable"] do* mutable,
            ["cpp:identifier:cpp_namespace"] break* namespace,
            ["cpp:identifier:cpp_not"] switch not,
            ["cpp:identifier:cpp_or"] int or
        )
        throws return, sizeof;
    }

    ["cpp:identifier:cpp_template"]
    const int template = 0;
}
