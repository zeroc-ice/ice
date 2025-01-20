// Copyright (c) ZeroC, Inc.

#ifndef TEST_I_H
#define TEST_I_H

#include "Test.h"

class TestIntfI final : public Test::TestIntf
{
public:
    void ping(std::optional<Test::PingReplyPrx>, const Ice::Current&) final;
    void sendByteSeq(Test::ByteSeq, std::optional<Test::PingReplyPrx>, const Ice::Current&) final;
    void pingBiDir(Ice::Identity, const Ice::Current&) final;
    void shutdown(const Ice::Current&) final;
};

#endif
