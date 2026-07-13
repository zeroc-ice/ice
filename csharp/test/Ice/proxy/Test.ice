// Copyright (c) ZeroC, Inc.

#pragma once

#include "Ice/Context.ice"

["cs:identifier:Ice.proxy.Test"]
module Test
{
    interface MyInterface
    {
        void shutdown();

        Ice::Context getContext();
    }

    interface MyDerivedInterface extends MyInterface
    {
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

    ["cs:internal"]
    interface C extends A, B
    {
        C* opC(C* c);
        S opS(S s);
    }
}
