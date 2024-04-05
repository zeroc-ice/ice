//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_I_H
#define TEST_I_H

#include "Test.h"

class TestIntfI final : public Test::TestIntf
{
public:
    void init(const Ice::Current&) final;

    std::int32_t getHeartbeatCount(const Ice::Current&) final { return _heartbeatCount; }
    void shutdown(const Ice::Current&) final;

    void incrementHeartbeatCount() { ++_heartbeatCount; }

private:
    std::int32_t _heartbeatCount = 0;
};
#endif