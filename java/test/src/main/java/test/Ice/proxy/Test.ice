//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

#include <Ice/Context.ice>

[[java:package(test.Ice.proxy)]]
module Test
{

interface MyClass
{
    void shutdown();

    Ice::Context getContext();
}

interface MyDerivedClass : MyClass
{
    Object* echo(Object* obj);
}

}
