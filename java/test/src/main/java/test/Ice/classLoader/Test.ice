// Copyright (c) ZeroC, Inc.

#pragma once

[["java:package:test.Ice.classLoader"]]
module Test
{
    class ConcreteClass
    {
        int i;
    }

    exception E {}

    interface Initial
    {
        ConcreteClass getConcreteClass();
        void throwException() throws E;
        void shutdown();
    }
}
