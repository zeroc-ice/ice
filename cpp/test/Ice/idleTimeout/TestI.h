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
    void setCallback(std::optional<Test::TestIntfPrx>, const Ice::Current&) final;
};
#endif
