// Copyright (c) ZeroC, Inc.

#pragma once

[["java:package:test.Slice.escape"]]

// TODO: figure out a better way to map module names.
module java_abstract
{

["java:identifier:_assert"]
enum assert
{
    ["java:identifier:java_boolean"] boolean
}

["java:identifier:_break"]
struct break
{
    ["java:identifier:java_case"] int case;
}

["java:identifier:_catch"]
interface catch
{
    ["java:identifier:myCheckedCast"]
    ["amd"] void checkedCast(
        ["java:identifier:java_clone"] int clone,
        out ["java:identifier:java_continue"] int continue
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
    ["java:identifier:java_native"] string native;
}

["java:identifier:_finalize"]
interface finalize extends default, catch
{
    ["java:identifier:_notify"]
    assert notify(
        ["java:identifier:java_notifyAll"] break notifyAll,
        ["java:identifier:java_null"] notify null,
        ["java:identifier:java_package"] finalize* package,
        ["java:identifier:java_return"] default* return,
        ["java:identifier:java_super"] int super
    ) throws hashCode, clone;
}

["java:identifier:java_for"] sequence<assert> for;
["java:identifier:java_goto"] dictionary<string, assert> goto;

["java:identifier:java_synchronized"] const int synchronized = 0;

}
