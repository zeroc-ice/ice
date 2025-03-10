// Copyright (c) ZeroC, Inc.

// TODO find a better solution for remapping modules.
module escapedAnd
{
    ["python:identifier:_assert"]
    enum assert
    {
        ["python:identifier:_break"]
        break
    }

    ["python:identifier:_continue"]
    struct continue
    {
        ["python:identifier:_def"]
        int def;
    }

    ["python:identifier:_del"]
    interface del
    {
        ["amd"] ["python:identifier:_elif"] void elif(
            ["python:identifier:_else"] int else,
            out ["python:identifier:_except"] int except
        );
    }

    ["python:identifier:_exec"]
    interface exec
    {
        ["python:identifier:_finally"]
        void finally();
    }

    ["python:identifier:_for"]
    class for
    {
        int foo;
        ["python:identifier:_from"] exec* from;
    }

    ["python:identifier:_is"]
    exception is
    {
        int bar;
    }

    ["python:identifier:_not"]
    exception not extends is
    {
        ["python:identifier:_pass"] int pass;
    }

    ["python:identifier:_if"]
    interface if extends exec, del
    {
        ["python:identifier:_raise"]
        assert raise(
            ["python:identifier:_else"] continue else,
            ["python:identifier:_return"] for return,
            ["python:identifier:_while"] del* while,
            ["python:identifier:_yield"] exec* yield,
            ["python:identifier:_or"] if* or,
            ["python:identifier:_global"] int global
        ) throws is;
    }

    ["python:identifier:_import"]
    sequence<assert> import;

    ["python:identifier:_in"]
    dictionary<string,assert> in;

    ["python:identifier:_lambda"]
    const int lambda = 0;

    enum EnumNone
    {
        ["python:identifier:_None"]
        None
    }
}
