//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

#include <Ice/Context.ice>

[[suppress-warning:reserved-identifier]]

module ZeroC::Ice::Test::Proxy
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
