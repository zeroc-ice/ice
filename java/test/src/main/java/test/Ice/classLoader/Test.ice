// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#pragma once

[["java:package:test.Ice.classLoader"]]
module Test
{

class ConcreteClass
{
    int i;
}

exception E {}

interface Initial
{
    ConcreteClass getConcreteClass();
    void throwException() throws E;
    void shutdown();
}

}
