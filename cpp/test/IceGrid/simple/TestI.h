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

    void shutdown(const Ice::Current&) override;
};

#endif
