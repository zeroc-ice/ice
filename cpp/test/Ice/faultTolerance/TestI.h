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

class TestI : public Test
{
public:

    TestI(const Ice::ObjectAdapterPtr&);

    virtual void shutdown(const Ice::Current&);
    virtual void abort(const Ice::Current&);
    virtual void idempotentAbort(const Ice::Current&);
    virtual void nonmutatingAbort(const Ice::Current&) const;
    virtual Ice::Int pid(const Ice::Current&);

private:

    Ice::ObjectAdapterPtr _adapter;
};

#endif
