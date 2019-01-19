//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

#include <Ice/Current.ice>

["objc:prefix:TestProxy"]
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
