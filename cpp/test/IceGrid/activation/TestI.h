// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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

#endif
