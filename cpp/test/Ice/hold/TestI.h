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

class HoldI : public Test::Hold
{
    virtual void putOnHold(Ice::Int, const Ice::Current&);
    virtual void shutdown(const Ice::Current&);
};

#endif
