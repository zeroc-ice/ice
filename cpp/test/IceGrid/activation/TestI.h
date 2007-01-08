// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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

    TestI(const Ice::ObjectAdapterPtr&);

    virtual void fail(const Ice::Current&);
    virtual void shutdown(const Ice::Current&);

    bool isFailed() const;

private:

    Ice::ObjectAdapterPtr _adapter;
    bool _failed;
};

#endif
