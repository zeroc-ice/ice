// Copyright (c) ZeroC, Inc.

#pragma once

[["java:package:test.Slice.generation"]]

module Test
{
    interface Interface1
    {
        void method();
    }
}

["java:package:test.Slice.generation.modpkg"]
module Test2
{
    class Class1 {}
}
