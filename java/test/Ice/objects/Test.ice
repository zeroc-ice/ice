// **********************************************************************
//
// Copyright (c) 2001
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

class B;
class C;

class A
{
    B theB;
    C theC;
};

class B extends A
{
    A theA;
};

class C
{
    B theB;
};

class D
{
    A theA;
    B theB;
    C theC;    
};

class Initial
{
    void shutdown();
    B getB1();
    B getB2();
    C getC();
    D getD();
    void getAll(out B b1, out B b2, out C theC, out D theD);
    void addFacetsToB1();
};

#endif
