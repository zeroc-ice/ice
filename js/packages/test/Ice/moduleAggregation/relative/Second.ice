// Copyright (c) ZeroC, Inc.

#pragma once

module Outer
{
    module Inner
    {
        struct Second
        {
            string name;
        }

        module Deep
        {
            struct DeepSecond
            {
                string deepName;
            }
        }
    }
}
