// Copyright (c) ZeroC, Inc.

["cpp:identifier:escaped_and"]
module and
{
    ["cpp:identifier:escaped_continue"]
    enum continue
    {
        ["cpp:identifier:escaped_asm"]
        asm
    }

    ["cpp:identifier:escaped_auto"]
    struct auto
    {
        ["cpp:identifier:escaped_default"]
        int default;
    }

    ["cpp:identifier:escaped_break"]
    interface break
    {
        ["amd", "cpp:identifier:escaped_case"] void case(
            ["cpp:identifier:escaped_catch"] int catch,
            ["cpp:identifier:escaped_try"] out int try
        );

        ["cpp:identifier:escaped_explicit"] void explicit();
    }

    ["cpp:identifier:escaped_switch"]
    class switch
    {
        ["cpp:identifier:escaped_if"] int if;
        ["cpp:identifier:escaped_export"] break* export;
        ["cpp:identifier:escaped_volatile"] int volatile;
    }

    ["cpp:identifier:escaped_do"]
    interface do extends break
    {
    }

    ["cpp:identifier:escaped_extern"]
    sequence<auto> extern;

    ["cpp:identifier:escaped_for"]
    dictionary<string,auto> for;

    ["cpp:identifier:escaped_return"]
    exception return
    {
        ["cpp:identifier:escaped_signed"]
        int signed;
    }

    ["cpp:identifier:escaped_sizeof"]
    exception sizeof extends return
    {
        ["cpp:identifier:escaped_static"] int static;
        ["cpp:identifier:escaped_switch"] int switch;
    }

    ["cpp:identifier:escaped_friend"]
    interface friend
    {
        ["cpp:identifier:escaped_goto"]
        auto goto(
            ["cpp:identifier:escaped_if"] continue if,
            ["cpp:identifier:escaped_d"] auto d,
            ["cpp:identifier:escaped_private"] switch private,
            ["cpp:identifier:escaped_mutable"] do* mutable,
            ["cpp:identifier:escaped_namespace"] break* namespace,
            ["cpp:identifier:escaped_not"] switch not,
            ["cpp:identifier:escaped_or"] int or
        )
        throws return, sizeof;
    }

    ["cpp:identifier:escaped_template"]
    const int template = 0;
}
