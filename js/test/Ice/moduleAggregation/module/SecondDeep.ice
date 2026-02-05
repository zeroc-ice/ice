// Copyright (c) ZeroC, Inc.

#pragma once

[["js:module:test_nested_modules"]]

module Outer
{
    module Inner
    {
        module Deep
        {
            struct DeepSecond
            {
                string deepName;
            }
        }
    }
}
