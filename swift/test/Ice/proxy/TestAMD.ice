//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

#include <Ice/Context.ice>

module Test
{

[amd] interface MyClass
{
    void shutdown();

    Ice::Context getContext();
}

[amd] interface MyDerivedClass : MyClass
{
    Object* echo(Object* obj);
}

}
