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

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class BI : public B
{
public:

    BI();

    virtual bool postUnmarshalInvoked(const Ice::Current&);

    virtual void ice_preMarshal();
    virtual void ice_postUnmarshal();

private:

    bool _postUnmarshalInvoked;
};

class CI : public C
{
public:

    CI();

    virtual bool postUnmarshalInvoked(const Ice::Current&);

    virtual void ice_preMarshal();
    virtual void ice_postUnmarshal();

private:

    bool _postUnmarshalInvoked;
};

class DI : public D
{
public:

    DI();

    virtual bool postUnmarshalInvoked(const Ice::Current&);

    virtual void ice_preMarshal();
    virtual void ice_postUnmarshal();

private:

    bool _postUnmarshalInvoked;
};

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
