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

private:

    Ice::ObjectAdapterPtr _adapter;
    BPtr _b1;
    BPtr _b2;
    CPtr _c;
    DPtr _d;
};

#endif
