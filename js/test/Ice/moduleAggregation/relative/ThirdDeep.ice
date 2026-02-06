// Copyright (c) ZeroC, Inc.

#pragma once

// This file defines ONLY a deeper module (Outer.Inner.Deep.DeepThird).
// It does NOT define anything directly in Outer.Inner.
// This tests that transitive includes can add to deeper modules
// even when the direct include (Third.ice) doesn't define that level.

module Outer
{
    module Inner
    {
        module Deep
        {
            struct DeepThird
            {
                int deepThirdValue;
            }
        }
    }
}
