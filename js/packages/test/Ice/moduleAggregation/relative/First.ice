// Copyright (c) ZeroC, Inc.

#pragma once

#include "FirstDeep.ice"

module Outer
{
    module Inner
    {
        struct First
        {
            int value;
        }

        module Deep
        {
            struct DeepFirst
            {
                int deepValue;
            }
        }
    }
}
