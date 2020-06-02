//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

module ZeroC::Ice::Test::Invoke
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
