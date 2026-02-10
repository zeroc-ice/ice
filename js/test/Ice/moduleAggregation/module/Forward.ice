// Copyright (c) ZeroC, Inc.

#pragma once

[["js:module:test_nested_modules"]]

// This file defines ForwardDeclared which will be forward declared in Test.ice
// with js:defined-in metadata. This tests that forward declared types are
// imported from the defining file, not double-exported.

module Outer
{
    module Inner
    {
        class ForwardDeclared
        {
            string forwardValue;
        }
    }
}
