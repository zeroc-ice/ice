//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_I_H
#define TEST_I_H

#include "Test.h"

class TestIntfI final : public Test::TestIntf
{
public:
    void sleep(std::int32_t, const Ice::Current&) final;

    void shutdown(const Ice::Current&) final;
};

class TestIntfBidirI final : public Test::TestIntfBidir
{
public:
    void putAsSleep(bool, std::int32_t, std::optional<Test::DelayedTestIntfPrx>, const Ice::Current&) final;
};
#endif
