// Copyright (c) ZeroC, Inc.

#pragma once

module Test
{
    interface MyClass
    {
        void incCounter(int expected);
        void waitCounter(int value);
        void shutdown();
    }
}
