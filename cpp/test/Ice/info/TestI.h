// Copyright (c) ZeroC, Inc.

#ifndef TEST_I_H
#define TEST_I_H

#include "Test.h"

class TestI final : public Test::TestIntf
{
public:
    void shutdown(const Ice::Current&) final;

    Ice::Context getEndpointInfoAsContext(const Ice::Current&) final;
    Ice::Context getConnectionInfoAsContext(const Ice::Current&) final;
};

#endif
