// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef TEST_ICE
#define TEST_ICE

module M_A
{

interface I_A
{
    I_A* ia(I_A* p);
};

class C_A
{
    C_A* ca(C_A* p);
};

};

module M_B
{

interface I_B1 extends M_A::I_A
{
    I_B1* ib1(I_B1* p);
};

interface I_B2 extends M_A::I_A
{
    I_B2* ib2(I_B2* p);
};

class C_B extends M_A::C_A
{
    C_B* cb(C_B* p);
};

};

module M_A
{

interface I_C extends M_B::I_B1, M_B::I_B2
{
    I_C* ic(I_C* p);
};

class C_C extends M_B::C_B
{
    C_C* cc(C_C* p);
};

class C_D extends C_C implements M_B::I_B1, M_B::I_B2
{
    C_D* cd(C_D* p);
};

};

interface Initial
{
    void shutdown();
    M_A::C_A* c_a();
    M_B::C_B* c_b();
    M_A::C_C* c_c();
    M_A::C_D* c_d();
    M_A::I_A* i_a();
    M_B::I_B1* i_b1();
    M_B::I_B2* i_b2();
    M_A::I_C* i_c();
};

#endif
