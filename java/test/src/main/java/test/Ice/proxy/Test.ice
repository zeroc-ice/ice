// Copyright (c) ZeroC, Inc.

#pragma once

#include "Ice/Context.ice"

[["java:package:test.Ice.proxy"]]
module Test
{
    interface MyClass
    {
        void shutdown();

        Ice::Context getContext();
    }

    interface MyDerivedClass extends MyClass
    {
        Object* echo(Object* obj);
    }

    interface MyOtherDerivedClass extends MyClass {}

    interface DiamondClass extends MyDerivedClass, MyOtherDerivedClass {}
}
