// Copyright (c) ZeroC, Inc.

#pragma once

[["java:package:test.Slice.escape"]]

["java:identifier:escaped_abstract"]
module abstract
{
    ["java:identifier:_assert"]
    enum assert
    {
        ["java:identifier:escaped_boolean"] boolean
    }

    ["java:identifier:_break"]
    struct break
    {
        ["java:identifier:escaped_case"] int case;
    }

    ["java:identifier:_catch"]
    interface catch
    {
        ["java:identifier:myCheckedCast"]
        ["amd"] void checkedCast(
            ["java:identifier:escaped_clone"] int clone,
            ["java:identifier:escaped_continue"] out int continue
        );
    }

    ["java:identifier:_default"]
    interface default
    {
        ["java:identifier:_do"]
        void do();
    }

    class notify
    {
        ["java:identifier:foo"] int if;
        ["java:identifier:_equals"] default* equals;
    }

    exception hashCode
    {
        ["java:identifier:bar"] int if;
    }

    exception clone extends hashCode
    {
        ["java:identifier:escaped_native"] string native;
    }

    ["java:identifier:_finalize"]
    interface finalize extends default, catch
    {
        ["java:identifier:_notify"]
        assert notify(
            ["java:identifier:escaped_notifyAll"] break notifyAll,
            ["java:identifier:escaped_null"] notify null,
            ["java:identifier:escaped_package"] finalize* package,
            ["java:identifier:escaped_return"] default* return,
            ["java:identifier:escaped_super"] int super
            ) throws hashCode, clone;
    }

    ["java:identifier:escaped_for"] sequence<assert> for;
    ["java:identifier:escaped_goto"] dictionary<string, assert> goto;

    ["java:identifier:escaped_synchronized"] const int synchronized = 0;
}
