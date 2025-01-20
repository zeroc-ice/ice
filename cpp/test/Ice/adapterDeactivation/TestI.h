// Copyright (c) ZeroC, Inc.

#ifndef TEST_I_H
#define TEST_I_H

#include "Test.h"

class TestI : public Test::TestIntf
{
public:
    void transient(const Ice::Current&) override;
    void deactivate(const Ice::Current&) override;
};

class Cookie
{
public:
    [[nodiscard]] std::string message() const;
};

#endif
