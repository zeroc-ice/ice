// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class InitialI : public Initial
{
public:

    InitialI(const Ice::ObjectAdapterPtr&);

    virtual void shutdown(const Ice::Current&);

    virtual M_A::C_APrx c_a(const Ice::Current&);
    virtual M_B::C_BPrx c_b(const Ice::Current&);
    virtual M_A::C_CPrx c_c(const Ice::Current&);
    virtual M_A::C_DPrx c_d(const Ice::Current&);
    virtual M_A::I_APrx i_a(const Ice::Current&);
    virtual M_B::I_B1Prx i_b1(const Ice::Current&);
    virtual M_B::I_B2Prx i_b2(const Ice::Current&);
    virtual M_A::I_CPrx i_c(const Ice::Current&);

private:

    Ice::ObjectAdapterPtr _adapter;
    M_A::C_APrx _ca;
    M_B::C_BPrx _cb;
    M_A::C_CPrx _cc;
    M_A::C_DPrx _cd;
    M_A::I_APrx _ia;
    M_B::I_B1Prx _ib1;
    M_B::I_B2Prx _ib2;
    M_A::I_CPrx _ic;
};

class C_A_I : virtual public M_A::C_A
{
public:

    virtual M_A::C_APrx ca(const M_A::C_APrx&, const Ice::Current&);
};

class C_B_I : virtual public M_B::C_B, virtual public C_A_I
{
public:

    virtual M_B::C_BPrx cb(const M_B::C_BPrx&, const Ice::Current&);
};

class C_C_I : virtual public M_A::C_C, virtual public C_B_I
{
public:

    virtual M_A::C_CPrx cc(const M_A::C_CPrx&, const Ice::Current&);
};

class I_A_I : virtual public M_A::I_A
{
public:

    virtual M_A::I_APrx ia(const M_A::I_APrx&, const Ice::Current&);
};

class I_B1_I : virtual public M_B::I_B1, virtual public I_A_I
{
public:

    virtual M_B::I_B1Prx ib1(const M_B::I_B1Prx&, const Ice::Current&);		   
};

class I_B2_I : virtual public M_B::I_B2, virtual public I_A_I
{
public:

    virtual M_B::I_B2Prx ib2(const M_B::I_B2Prx&, const Ice::Current&);
};

class I_C_I : virtual public M_A::I_C, virtual public I_B1_I, virtual public I_B2_I
{
public:

    virtual M_A::I_CPrx ic(const M_A::I_CPrx&, const Ice::Current&);
};

class C_D_I : virtual public M_A::C_D, virtual public C_C_I, virtual public I_B1_I, virtual public I_B2_I
{
public:

    virtual M_A::C_DPrx cd(const M_A::C_DPrx&, const Ice::Current&);
};

#endif
