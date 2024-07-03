// Copyright (c) ZeroC, Inc.

#ifndef TEST_I_H
#define TEST_I_H

#include "Test.h"

class MyObjectI final : public Test::MyObject
{
public:
    std::string getName(const Ice::Current&) final;
};

#endif
