// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef TEST_ICE
#define TEST_ICE

interface A
{
    string callA();
};

interface B extends A
{
    string callB();
};

interface C extends A
{
    string callC();
};

interface D extends B, C
{
    string callD();
};

interface E
{
    string callE();
};

interface F extends E
{
    string callF();
};

interface G
{
    void shutdown();
    string callG();
};

#endif
