// Copyright (c) ZeroC, Inc.

#pragma once

#include "Ice/Context.ice"

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
}
