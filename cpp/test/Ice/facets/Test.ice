// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef TEST_ICE
#define TEST_ICE

interface Empty
{
};

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

interface H extends G
{
    string callH();
};

#endif
