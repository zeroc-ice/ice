// Copyright (c) ZeroC, Inc.

#ifndef TEST_I_H
#define TEST_I_H

#include "Test.h"

bool endsWith(const std::string&, const std::string&);

class ThrowerI : public Test::Thrower
{
public:
    ThrowerI();

    void shutdown(const Ice::Current&) override;
    bool supportsUndeclaredExceptions(const Ice::Current&) override;
    bool supportsAssertException(const Ice::Current&) override;

    void throwAasA(std::int32_t, const Ice::Current&) override;
    void throwAorDasAorD(std::int32_t, const Ice::Current&) override;
    void throwBasA(std::int32_t, std::int32_t, const Ice::Current&) override;
    void throwCasA(std::int32_t, std::int32_t, std::int32_t, const Ice::Current&) override;
    void throwBasB(std::int32_t, std::int32_t, const Ice::Current&) override;
    void throwCasB(std::int32_t, std::int32_t, std::int32_t, const Ice::Current&) override;
    void throwCasC(std::int32_t, std::int32_t, std::int32_t, const Ice::Current&) override;

    void throwModA(std::int32_t, std::int32_t, const Ice::Current&) override;

    void throwUndeclaredA(std::int32_t, const Ice::Current&) override;
    void throwUndeclaredB(std::int32_t, std::int32_t, const Ice::Current&) override;
    void throwUndeclaredC(std::int32_t, std::int32_t, std::int32_t, const Ice::Current&) override;
    void throwLocalException(const Ice::Current&) override;
    void throwNonIceException(const Ice::Current&) override;
    void throwAssertException(const Ice::Current&) override;
    Ice::ByteSeq throwMemoryLimitException(Ice::ByteSeq, const Ice::Current&) override;

    void throwLocalExceptionIdempotent(const Ice::Current&) override;

    void throwDispatchException(std::uint8_t replyStatus, const Ice::Current&) override;

    void throwAfterResponse(const Ice::Current&) override;
    void throwAfterException(const Ice::Current&) override;
};

#endif
