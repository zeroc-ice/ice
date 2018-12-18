// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#pragma once

[["java:package:test.Ice.facets"]]
module Test
{

interface Empty
{
}

interface A
{
    string callA();
}

interface B extends A
{
    string callB();
}

interface C extends A
{
    string callC();
}

interface D extends B, C
{
    string callD();
}

interface E
{
    string callE();
}

interface F extends E
{
    string callF();
}

interface G
{
    void shutdown();
    string callG();
}

interface H extends G
{
    string callH();
}

}
