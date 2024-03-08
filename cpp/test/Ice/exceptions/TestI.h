//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

bool endsWith(const std::string&, const std::string&);

class ThrowerI : public Test::Thrower
{
public:
    ThrowerI();

    virtual void shutdown(const Ice::Current&);
    virtual bool supportsUndeclaredExceptions(const Ice::Current&);
    virtual bool supportsAssertException(const Ice::Current&);

    virtual void throwAasA(std::int32_t, const Ice::Current&);
    virtual void throwAorDasAorD(std::int32_t, const Ice::Current&);
    virtual void throwBasA(std::int32_t, std::int32_t, const Ice::Current&);
    virtual void throwCasA(std::int32_t, std::int32_t, std::int32_t, const Ice::Current&);
    virtual void throwBasB(std::int32_t, std::int32_t, const Ice::Current&);
    virtual void throwCasB(std::int32_t, std::int32_t, std::int32_t, const Ice::Current&);
    virtual void throwCasC(std::int32_t, std::int32_t, std::int32_t, const Ice::Current&);

    virtual void throwModA(std::int32_t, std::int32_t, const Ice::Current&);

    virtual void throwUndeclaredA(std::int32_t, const Ice::Current&);
    virtual void throwUndeclaredB(std::int32_t, std::int32_t, const Ice::Current&);
    virtual void throwUndeclaredC(std::int32_t, std::int32_t, std::int32_t, const Ice::Current&);
    virtual void throwLocalException(const Ice::Current&);
    virtual void throwNonIceException(const Ice::Current&);
    virtual void throwAssertException(const Ice::Current&);
    virtual Ice::ByteSeq throwMemoryLimitException(Ice::ByteSeq, const Ice::Current&);

    virtual void throwLocalExceptionIdempotent(const Ice::Current&);

    virtual void throwAfterResponse(const Ice::Current&);
    virtual void throwAfterException(const Ice::Current&);
};

#endif
