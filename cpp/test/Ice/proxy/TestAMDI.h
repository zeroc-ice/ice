// Copyright (c) ZeroC, Inc.

#ifndef TEST_AMD_I_H
#define TEST_AMD_I_H

#include "TestAMD.h"

class MyDerivedClassI : public Test::MyDerivedClass
{
public:
    MyDerivedClassI();

    void echoAsync(
        std::optional<Ice::ObjectPrx>,
        std::function<void(const std::optional<Ice::ObjectPrx>&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void shutdownAsync(std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&) override;

    void getContextAsync(
        std::function<void(const Ice::Context&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    [[nodiscard]] bool ice_isA(std::string, const Ice::Current&) const override;

private:
    mutable Ice::Context _ctx;
};

#endif
