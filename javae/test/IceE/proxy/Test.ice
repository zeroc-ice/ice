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

module Test
{

["java:type:java.util.Hashtable"] dictionary<string, string> Context;

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
