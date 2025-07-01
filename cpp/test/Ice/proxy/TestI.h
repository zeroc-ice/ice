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

class CI final : public Test::C
{
public:
    std::optional<Test::APrx> opA(std::optional<Test::APrx> a, const Ice::Current&) final { return a; }

    std::optional<Test::BPrx> opB(std::optional<Test::BPrx> b, const Ice::Current&) final { return b; }

    std::optional<Test::CPrx> opC(std::optional<Test::CPrx> c, const Ice::Current&) final { return c; }

    Test::S opS(Test::S s, const Ice::Current&) final { return s; }
};

#endif
