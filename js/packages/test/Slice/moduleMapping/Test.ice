// Copyright (c) ZeroC, Inc.

#pragma once

["js:identifier:acme.foo"]
module foo
{
    class One
    {
        string name;
    };

    ["js:identifier:baz.quux"]
    module baz
    {
        class One
        {
            string name;
        };
    }

    ["js:identifier:max_"]
    module max
    {
        class One
        {
            string name;
        };
    }
}
