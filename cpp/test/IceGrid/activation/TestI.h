//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class TestI final : public ::Test::TestIntf
{
public:

    TestI();

    void fail(const Ice::Current&) override;
    void shutdown(const Ice::Current&) override;

    bool isFailed() const;

private:

    bool _failed;
};

#endif
