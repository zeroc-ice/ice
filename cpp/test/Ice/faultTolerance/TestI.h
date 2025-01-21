// Copyright (c) ZeroC, Inc.

#ifndef TEST_I_H
#define TEST_I_H

#include "Test.h"

class TestI final : public Test::TestIntf
{
public:
    TestI();

    void shutdown(const Ice::Current&) final;
    void abort(const Ice::Current&) final;
    void idempotentAbort(const Ice::Current&) final;
    std::int32_t pid(const Ice::Current&) final;
};

#endif
