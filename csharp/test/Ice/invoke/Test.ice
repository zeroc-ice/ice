// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#pragma once

["cs:namespace:Ice.invoke"]
module Test
{

exception MyException
{
}

interface MyClass
{
    void opOneway();

    string opString(string s1, out string s2);

    void opException() throws MyException;

    void shutdown();
}

}
