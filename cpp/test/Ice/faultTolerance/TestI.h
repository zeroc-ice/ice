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

class TestI : public Test
{
public:

    TestI(const Ice::ObjectAdapterPtr&);

    virtual void shutdown();
    virtual void abort();
    virtual void nonmutatingAbort();
    virtual Ice::Int pid();

private:

    Ice::ObjectAdapterPtr _adapter;
};

#endif
