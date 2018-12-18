// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#pragma once

#include <Ice/Current.ice>

["cs:namespace:Ice.proxy.AMD"]
module Test
{

["amd"] interface MyClass
{
    void shutdown();

    Ice::Context getContext();
}

["amd"] interface MyDerivedClass extends MyClass
{
    Object* echo(Object* obj);
}

}
