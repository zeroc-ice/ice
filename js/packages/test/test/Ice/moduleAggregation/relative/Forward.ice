// Copyright (c) ZeroC, Inc.

#pragma once

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
