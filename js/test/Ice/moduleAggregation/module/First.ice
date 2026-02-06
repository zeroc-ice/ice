// Copyright (c) ZeroC, Inc.

#pragma once

[["js:module:test_nested_modules"]]

#include "FirstDeep.ice"

module Outer
{
    module Inner
    {
        struct First
        {
            int value;
        }
    }
}
