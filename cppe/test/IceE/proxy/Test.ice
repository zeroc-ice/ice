// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_ICE
#define TEST_ICE

module Test
{

dictionary<string, string> Context;

["ami"] class MyClass
{
    void shutdown();

    Context getContext();
};

["ami"] class MyDerivedClass extends MyClass
{
};

};

#endif
