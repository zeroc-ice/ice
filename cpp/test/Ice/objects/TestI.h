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

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class InitialI : public Initial
{
public:

    InitialI(const Ice::ObjectAdapterPtr&);

    virtual void shutdown(const Ice::Current&);
    virtual BPtr getB1(const Ice::Current&);
    virtual BPtr getB2(const Ice::Current&);
    virtual CPtr getC(const Ice::Current&);
    virtual DPtr getD(const Ice::Current&);
    virtual void getAll(BPtr&, BPtr&, CPtr&, DPtr&, const Ice::Current&);
    virtual void addFacetsToB1(const Ice::Current&);

private:

    Ice::ObjectAdapterPtr _adapter;
    BPtr _b1;
    BPtr _b2;
    CPtr _c;
    DPtr _d;
};

#endif
