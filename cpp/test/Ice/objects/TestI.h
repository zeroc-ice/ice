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

    virtual void shutdown();
    virtual BPtr getB1();
    virtual BPtr getB2();
    virtual CPtr getC();
    virtual DPtr getD();
    virtual void getAll(BPtr&, BPtr&, CPtr&, DPtr&);
    virtual void addFacetsToB1();

private:

    Ice::ObjectAdapterPtr _adapter;
    BPtr _b1;
    BPtr _b2;
    CPtr _c;
    DPtr _d;
};

#endif
