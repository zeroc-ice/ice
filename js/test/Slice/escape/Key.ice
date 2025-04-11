// Copyright (c) ZeroC, Inc.

#pragma once

["js:identifier:escapedAwait"]
module await
{
    ["js:identifier:_var"]
    enum var
    {
        base
    }

    ["js:identifier:_break"]
    struct break
    {
        ["js:identifier:_while"] int while;
        ["js:identifier:_constructor"] string constructor;
    }

    ["js:identifier:_case"]
    interface case
    {
        ["amd"] ["js:identifier:_catch"] void catch(int checked, ["js:identifier:_continue"] out int continue);
    }

    ["js:identifier:_typeof"]
    interface typeof
    {
        ["js:identifier:_default"]
        void default();
    }

    ["js:identifier:_delete"]
    class delete
    {
        ["js:identifier:_else"] case* else;
        ["js:identifier:_export"] int export;
        ["js:identifier:_clone"] string clone;
    }

    exception fixed
    {
        ["js:identifier:myFor"] int for;
    }

    exception foreach extends fixed
    {
        int goto;
        ["js:identifier:_if"] int if;
    }

    interface explicit extends typeof, case
    {
        ["js:identifier:_in"] var in(
            break internal,
            ["js:identifier:_new"] typeof* new,
            ["js:identifier:_null"] delete null,
            int override
        ) throws fixed, foreach;
    }

    ["js:identifier:_while"]
    dictionary<string, break> while;

    ["js:identifier:_package"]
    class package
    {
        ["js:identifier:_for"] optional(1) break for;
        optional(2) var goto;
        ["js:identifier:_null"] optional(3) explicit* null;
        optional(5) while internal;
        ["js:identifier:_debugger"] optional(7) string debugger;
    }

    interface optionalParams
    {
        ["js:identifier:_in"]
        optional(1) break in(optional(2) var goto,
            ["js:identifier:_if"] optional(3) explicit* if,
            optional(5) while internal,
            out optional(7) string namespace,
            ["js:identifier:_null"] out optional(8) explicit* null);

        ["amd"] ["js:identifier:_continue"]
        optional(1) break continue(optional(2) var goto,
            ["js:identifier:_if"] optional(3) explicit* if,
            optional(5) while internal,
            out optional(7) string namespace,
            ["js:identifier:_null"] out optional(8) explicit* null);
    }

    ["js:identifier:_public"]
    const int public = 0;

    // System as inner module.
    module System
    {
        interface Test
        {
            void op();
        }
    }
}

// System as outer module.
module System
{
    interface Test
    {
        void op();
    }
}
