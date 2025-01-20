// Copyright (c) ZeroC, Inc.

#ifndef TEST_I_H
#define TEST_I_H

#include "Test.h"

class MyDerivedClassI final : public Test::MyDerivedClass
{
public:
    MyDerivedClassI();

    std::optional<Ice::ObjectPrx> echo(std::optional<Ice::ObjectPrx>, const Ice::Current&) final;
    [[nodiscard]] bool ice_isA(std::string, const Ice::Current&) const final;
    void shutdown(const Ice::Current&) final;
    Ice::Context getContext(const Ice::Current&) final;

private:
    mutable Ice::Context _ctx;
};

#endif
