// Copyright (c) ZeroC, Inc.

#ifndef TEST_I_H
#define TEST_I_H

#include "Test.h"

class TestI : public ::Test::TestIntf
{
public:
    TestI();

    void shutdown(const Ice::Current&) override;
};

#endif
