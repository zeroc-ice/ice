// Copyright (c) ZeroC, Inc.

#pragma once

#include "Ice/Context.ice"

module Test
{
    interface MyClass
    {
        void shutdown();
<<<<<<< Updated upstream

        Ice::Context getContext();
    }

=======

        Ice::Context getContext();
    }

>>>>>>> Stashed changes
    interface MyDerivedClass extends MyClass
    {
        Object* echo(Object* obj);
    }
}
