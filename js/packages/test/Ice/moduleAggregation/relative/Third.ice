// Copyright (c) ZeroC, Inc.

#pragma once

// Third.ice defines Outer.Inner.Third but does NOT define Outer.Inner.Deep.
// The transitive include (ThirdDeep.ice) defines Outer.Inner.Deep.DeepThird.
// This tests that aggregation doesn't drop deeper modules when only
// a transitive include defines them.

#include "ThirdDeep.ice"

module Outer
{
    module Inner
    {
        struct Third
        {
            int thirdValue;
        }
    }
}
