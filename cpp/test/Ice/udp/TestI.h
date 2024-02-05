//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class TestIntfI : public Test::TestIntf
{
public:

    virtual void ping(Test::PingReplyPrxPtr, const Ice::Current&);
    virtual void sendByteSeq(Test::ByteSeq, Test::PingReplyPrxPtr, const Ice::Current&);
    virtual void pingBiDir(Ice::Identity, const Ice::Current&);
    virtual void shutdown(const Ice::Current&);
};

#endif
