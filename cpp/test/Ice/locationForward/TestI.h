// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
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

    TestI(const Ice::ObjectAdapterPtr&, const Ice::ObjectPrx);

    virtual void shutdown(const Ice::Current&);

private:

    Ice::ObjectAdapterPtr _adapter;
    Ice::ObjectPrx _fwd;
};

#endif
