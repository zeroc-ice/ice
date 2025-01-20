// Copyright (c) ZeroC, Inc.

#ifndef TESTAMD_I_H
#define TESTAMD_I_H

#include "TestAMD.h"

class TestAMDI : public Test::TestIntf
{
public:
    void requestFailedExceptionAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void unknownUserExceptionAsync(std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&)
        override;

    void unknownLocalExceptionAsync(std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&)
        override;

    void
    unknownExceptionAsync(std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&) override;

    void
    userExceptionAsync(std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&) override;

    void
    localExceptionAsync(std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&) override;

    void
    stdExceptionAsync(std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&) override;

    void
    cppExceptionAsync(std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&) override;

    void unknownExceptionWithServantExceptionAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void impossibleExceptionAsync(
        bool,
        std::function<void(std::string_view)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void intfUserExceptionAsync(
        bool,
        std::function<void(std::string_view)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void
    asyncResponseAsync(std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&) override;

    void
    asyncExceptionAsync(std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&) override;

    void shutdownAsync(std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&) override;
};

class Cookie
{
public:
    [[nodiscard]] std::string message() const;
};

#endif
