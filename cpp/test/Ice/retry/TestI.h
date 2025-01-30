// Copyright (c) ZeroC, Inc.

#ifndef TEST_I_H
#define TEST_I_H

#include "Test.h"

class RetryI : public Test::Retry
{
public:
    RetryI();

    void op(bool, const Ice::Current&) override;
    int opIdempotent(int, const Ice::Current&) override;
    void opNotIdempotent(const Ice::Current&) override;
    void sleep(int, const Ice::Current&) override;
    void shutdown(const Ice::Current&) override;

private:
    int _counter{0};
};

#endif
