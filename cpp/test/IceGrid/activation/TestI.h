// Copyright (c) ZeroC, Inc.

#ifndef TEST_I_H
#define TEST_I_H

#include "Test.h"

class TestI final : public Test::TestIntf
{
public:
    TestI();

    void fail(const Ice::Current&) override;
    void shutdown(const Ice::Current&) override;

    [[nodiscard]] bool isFailed() const;

private:
    bool _failed{false};
};
using TestIPtr = std::shared_ptr<TestI>;
#endif
