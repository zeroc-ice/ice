//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

#include <Ice/Context.ice>

["cs:namespace:Ice.proxy"]
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
