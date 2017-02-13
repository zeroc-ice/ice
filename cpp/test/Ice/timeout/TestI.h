// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class TimeoutI : virtual public Test::Timeout
{
public:

    virtual void op(const Ice::Current&);
    virtual void sendData(const Test::ByteSeq&, const Ice::Current&);
    virtual void sleep(Ice::Int, const Ice::Current&);
    virtual void holdAdapter(Ice::Int, const Ice::Current&);
    virtual void shutdown(const Ice::Current&);
};

#endif
