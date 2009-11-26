// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_ICE
#define TEST_ICE

module Test
{

exception MyException
{
};

class MyClass
{
    string opString(string s1, out string s2);

    void opException() throws MyException;

    void shutdown();
};

};

#endif
