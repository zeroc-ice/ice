// Copyright (c) ZeroC, Inc.

#pragma once

[["js:module:test_nested_modules"]]

module Outer
{
    module Inner
    {
        module Deep
        {
            struct DeepFirst
            {
                int deepValue;
            }
        }

        module Transitive
        {
            struct TransitiveFirst
            {
                int transitiveValue;
            }
        }
    }
}
