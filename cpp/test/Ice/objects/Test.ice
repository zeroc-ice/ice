// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
