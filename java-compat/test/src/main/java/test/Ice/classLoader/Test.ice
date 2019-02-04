//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[["java:package:test.Ice.classLoader"]]
[["suppress-warning:deprecated"]] // For classes with operations
module Test
{

class ConcreteClass
{
    int i;
}

class AbstractClass
{
    void op();
}

exception E {}

interface Initial
{
    ConcreteClass getConcreteClass();
    AbstractClass getAbstractClass();
    void throwException() throws E;
    void shutdown();
}

}
