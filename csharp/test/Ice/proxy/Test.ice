//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

#include <Ice/BuiltinSequences.ice>
#include <Ice/Context.ice>

[[suppress-warning(reserved-identifier)]]

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

    // Gets the location carried by this ice2 request.
    Ice::StringSeq getLocation();
}

}
