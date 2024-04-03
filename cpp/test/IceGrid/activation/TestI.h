//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_I_H
#define TEST_I_H

#include "Test.h"

class TestI final : public Test::TestIntf
{
public:
    TestI();

    virtual void fail(const Ice::Current&);
    virtual void shutdown(const Ice::Current&);

    bool isFailed() const;

private:
    bool _failed;
};
using TestIPtr = std::shared_ptr<TestI>;
#endif
