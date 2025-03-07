// Copyright (c) ZeroC, Inc.

#pragma once

module Test
{
    dictionary<string, string> Context;
<<<<<<< Updated upstream

    interface MyClass
    {
        void shutdown();

        Context getContext();
    }

=======

    interface MyClass
    {
        void shutdown();

        Context getContext();
    }

>>>>>>> Stashed changes
    interface MyDerivedClass extends MyClass
    {
        Object* echo(Object* obj);
    }
}
