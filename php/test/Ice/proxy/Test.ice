// Copyright (c) ZeroC, Inc.

#pragma once

module Test
{
    dictionary<string, string> Context;

    interface MyClass
    {
        void shutdown();

        Context getContext();
    }

    interface MyDerivedClass extends MyClass
    {
        ["php:identifier:_echo"]
        Object* echo(Object* obj);
    }

    interface A
    {
        A* opA(A* a);
    }

    interface B
    {
        B* opB(B* b);
    }

    struct S
    {
        A* a;
        B* b;
    }

    interface C extends A, B
    {
        C* opC(C* c);
        S opS(S s);
    }
}
