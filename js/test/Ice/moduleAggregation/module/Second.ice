// Copyright (c) ZeroC, Inc.

#pragma once

[["js:module:test_nested_modules"]]

#include "SecondDeep.ice"

module Outer
{
    module Inner
    {
        struct Second
        {
            string name;
        }
    }
}
