// Copyright (c) ZeroC, Inc.

#pragma once

["java:identifier:test.Ice.classLoader.Test"]
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
