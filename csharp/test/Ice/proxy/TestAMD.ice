// Copyright (c) ZeroC, Inc.

#pragma once

#include "Ice/Context.ice"

["cs:namespace:Ice.proxy.AMD"]
module Test
{
    ["amd"] interface MyClass
    {
        void shutdown();
<<<<<<< Updated upstream

        Ice::Context getContext();
    }

=======

        Ice::Context getContext();
    }

>>>>>>> Stashed changes
    ["amd"] interface MyDerivedClass extends MyClass
    {
        Object* echo(Object* obj);
    }
}
