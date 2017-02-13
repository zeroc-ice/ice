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
#include <TestCommon.h>

class TestIntfI : public Test::TestIntf
{
public:

    virtual void ping(ICE_IN(Test::PingReplyPrxPtr), const Ice::Current&);
    virtual void sendByteSeq(ICE_IN(Test::ByteSeq), ICE_IN(Test::PingReplyPrxPtr), const Ice::Current&);
    virtual void pingBiDir(ICE_IN(Ice::Identity), const Ice::Current&);
    virtual void shutdown(const Ice::Current&);
};

#endif
