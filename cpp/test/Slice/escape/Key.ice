// Copyright (c) ZeroC, Inc.

// TODO figure out how to escape identifiers on modules!
//["cpp:identifier:_cpp_and"] This should be `module and` which is mapped to `_cpp_and`.
module cpp_and
{

["cpp:identifier:_cpp_continue"]
enum continue
{
    ["cpp:identifier:_cpp_asm"]
    asm
}

["cpp:identifier:_cpp_auto"]
struct auto
{
    ["cpp:identifier:_cpp_default"]
    int default;
}

["cpp:identifier:_cpp_break"]
interface break
{
    ["amd", "cpp:identifier:_cpp_case"] void case(
        ["cpp:identifier:_cpp_catch"] int catch,
        out ["cpp:identifier:_cpp_try"] int try
    );

    ["cpp:identifier:_cpp_explicit"] void explicit();
}

["cpp:identifier:_cpp_switch"]
class switch
{
    ["cpp:identifier:_cpp_if"] int if;
    ["cpp:identifier:_cpp_export"] break* export;
    ["cpp:identifier:_cpp_volatile"] int volatile;
}

["cpp:identifier:_cpp_do"]
interface do extends break
{
}

["cpp:identifier:_cpp_extern"]
sequence<auto> extern;

["cpp:identifier:_cpp_for"]
dictionary<string,auto> for;

["cpp:identifier:_cpp_return"]
exception return
{
    ["cpp:identifier:_cpp_signed"]
    int signed;
}

["cpp:identifier:_cpp_sizeof"]
exception sizeof extends return
{
    ["cpp:identifier:_cpp_static"] int static;
    ["cpp:identifier:_cpp_switch"] int switch;
}

["cpp:identifier:_cpp_friend"]
interface friend
{
    ["cpp:identifier:_cpp_goto"]
    auto goto(
        ["cpp:identifier:_cpp_if"] continue if,
        ["cpp:identifier:_cpp_d"] auto d,
        ["cpp:identifier:_cpp_private"] switch private,
        ["cpp:identifier:_cpp_mutable"] do* mutable,
        ["cpp:identifier:_cpp_namespace"] break* namespace,
        ["cpp:identifier:_cpp_not"] switch not,
        ["cpp:identifier:_cpp_or"] int or
    )
    throws return, sizeof;
}

["cpp:identifier:_cpp_template"]
const int template = 0;

}
