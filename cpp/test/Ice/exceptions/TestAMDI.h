// Copyright (c) ZeroC, Inc.

#ifndef TEST_I_H
#define TEST_I_H

#include "Test.h"

bool endsWith(const std::string&, const std::string&);

class ThrowerI : public Test::AsyncThrower
{
public:
    ThrowerI();

    void shutdownAsync(std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&) override;

    void supportsUndeclaredExceptionsAsync(
        std::function<void(bool)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void supportsAssertExceptionAsync(
        std::function<void(bool)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void
    throwAasAAsync(int, std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&) override;

    void throwAorDasAorDAsync(int, std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&)
        override;

    void throwBasAAsync(int, int, std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&)
        override;

    void
    throwCasAAsync(int, int, int, std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&)
        override;

    void throwBasBAsync(int, int, std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&)
        override;

    void
    throwCasBAsync(int, int, int, std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&)
        override;

    void
    throwCasCAsync(int, int, int, std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&)
        override;

    void throwModAAsync(int, int, std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&)
        override;

    void throwUndeclaredAAsync(int, std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&)
        override;

    void
    throwUndeclaredBAsync(int, int, std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&)
        override;

    void throwUndeclaredCAsync(
        int,
        int,
        int,
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void throwLocalExceptionAsync(std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&)
        override;

    void throwNonIceExceptionAsync(std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&)
        override;

    void throwAssertExceptionAsync(std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&)
        override;

    void throwMemoryLimitExceptionAsync(
        Ice::ByteSeq,
        std::function<void(const Ice::ByteSeq&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void throwLocalExceptionIdempotentAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void throwDispatchExceptionAsync(
        std::uint8_t replyStatus,
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&) override;

    void throwAfterResponseAsync(std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&)
        override;

    void throwAfterExceptionAsync(std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&)
        override;

    void throwEAsync(std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&) override;
};

#endif
