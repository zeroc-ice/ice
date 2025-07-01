// Copyright (c) ZeroC, Inc.

#pragma once

#include "Ice/Context.ice"

["java:identifier:test.Ice.proxy.Test"]
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
