// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

#include<Ice/Current.ice>

[["java:package:test.Ice.proxy.AMD"]]
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
