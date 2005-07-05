// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class TestI : public Test::TestIntf
{
public:

    TestI(const IceE::ObjectAdapterPtr&);

    virtual void shutdown(const IceE::Current&);
    virtual void abort(const IceE::Current&);
    virtual void idempotentAbort(const IceE::Current&);
    virtual void nonmutatingAbort(const IceE::Current&) const;
    virtual IceE::Int pid(const IceE::Current&);

private:

    IceE::ObjectAdapterPtr _adapter;
};

#endif
