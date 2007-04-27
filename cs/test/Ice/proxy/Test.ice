// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_ICE
#define TEST_ICE

#include <Ice/Current.ice>

module Test
{


["ami"] class MyClass
{
    void shutdown();

    void opSleep(int duration);
    
    Ice::Context getContext();
};

["ami"] class MyDerivedClass extends MyClass
{
};

};

#endif
