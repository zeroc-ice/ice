//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[[suppress-warning(reserved-identifier)]]

module ZeroC::Ice::Test::Facets
{

interface Empty
{
}

interface A
{
    string callA();
}

interface B : A
{
    string callB();
}

interface C : A
{
    string callC();
}

interface D : B, C
{
    string callD();
}

interface E
{
    string callE();
}

interface F : E
{
    string callF();
}

interface G
{
    void shutdown();
    string callG();
}

interface H : G
{
    string callH();
}

}
