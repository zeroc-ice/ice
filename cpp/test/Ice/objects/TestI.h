// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class BI : public Test::B
{
public:

    BI();

    virtual bool postUnmarshalInvoked(const Ice::Current&);

    virtual void ice_preMarshal();
    virtual void ice_postUnmarshal();

private:

    bool _postUnmarshalInvoked;
};

class CI : public Test::C
{
public:

    CI();

    virtual bool postUnmarshalInvoked(const Ice::Current&);

    virtual void ice_preMarshal();
    virtual void ice_postUnmarshal();

private:

    bool _postUnmarshalInvoked;
};

class DI : public Test::D
{
public:

    DI();

    virtual bool postUnmarshalInvoked(const Ice::Current&);

    virtual void ice_preMarshal();
    virtual void ice_postUnmarshal();

private:

    bool _postUnmarshalInvoked;
};

class InitialI : public Test::Initial
{
public:

    InitialI(const Ice::ObjectAdapterPtr&);

    virtual void shutdown(const Ice::Current&);
    virtual Test::BPtr getB1(const Ice::Current&);
    virtual Test::BPtr getB2(const Ice::Current&);
    virtual Test::CPtr getC(const Ice::Current&);
    virtual Test::DPtr getD(const Ice::Current&);
    virtual void getAll(Test::BPtr&, Test::BPtr&, Test::CPtr&, Test::DPtr&, const Ice::Current&);

private:

    Ice::ObjectAdapterPtr _adapter;
    Test::BPtr _b1;
    Test::BPtr _b2;
    Test::CPtr _c;
    Test::DPtr _d;
};

#endif
