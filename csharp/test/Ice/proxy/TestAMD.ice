// Copyright (c) ZeroC, Inc.

#pragma once

#include "Ice/Context.ice"

["cs:identifier:Ice.proxy.AMD.Test"]
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
}
