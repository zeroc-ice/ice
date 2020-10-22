//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[[suppress-warning(reserved-identifier)]]

module ZeroC::Ice::Test::Invoke
{

exception MyException
{
}

interface MyClass
{
    void opOneway();

    (string r1, string r2) opString(string s1);

    void opException();

    void shutdown();
}

}
