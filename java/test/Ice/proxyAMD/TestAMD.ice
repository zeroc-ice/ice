// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_AMD_ICE
#define TEST_AMD_ICE

#include<Ice/Current.ice>

module Test
{

["ami", "amd"] class MyClass
{
    void shutdown();

    Ice::Context getContext();
};

["ami", "amd"] class MyDerivedClass extends MyClass
{
    Object* echo(Object* obj);
};

};

#endif
