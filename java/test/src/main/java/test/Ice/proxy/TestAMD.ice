// Copyright (c) ZeroC, Inc.

#pragma once

#include"Ice/Context.ice"

[["java:package:test.Ice.proxy.AMD"]]
module Test
{
    ["amd"] interface MyClass
    {
        void shutdown();

        Ice::Context getContext();
    }

    ["amd"] interface MyDerivedClass extends MyClass
    {
        Object* echo(Object* obj);
    }

    ["amd"] interface MyOtherDerivedClass extends MyClass {}

    ["amd"] interface DiamondClass extends MyDerivedClass, MyOtherDerivedClass {}
}
