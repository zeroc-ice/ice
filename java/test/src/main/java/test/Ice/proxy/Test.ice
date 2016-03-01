// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

#include <Ice/Current.ice>

[["java:package:test.Ice.proxy"]]
module Test
{

class MyClass
{
    void shutdown();

    Ice::Context getContext();
};

class MyDerivedClass extends MyClass
{
    Object* echo(Object* obj);
};

};
