// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
