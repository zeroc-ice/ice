// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class TestI : public ::Test::TestIntf
{
public:

    TestI();

    virtual void fail(const Ice::Current&);
    virtual void shutdown(const Ice::Current&);

    bool isFailed() const;

private:

    bool _failed;
};
ICE_DEFINE_PTR(TestIPtr, TestI);
#endif
