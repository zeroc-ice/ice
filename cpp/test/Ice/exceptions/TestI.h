// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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

    virtual void throwAasA(Ice::Int, const Ice::Current&);
    virtual void throwAorDasAorD(Ice::Int, const Ice::Current&);
    virtual void throwBasA(Ice::Int, Ice::Int, const Ice::Current&);
    virtual void throwCasA(Ice::Int, Ice::Int, Ice::Int, const Ice::Current&);
    virtual void throwBasB(Ice::Int, Ice::Int, const Ice::Current&);
    virtual void throwCasB(Ice::Int, Ice::Int, Ice::Int, const Ice::Current&);
    virtual void throwCasC(Ice::Int, Ice::Int, Ice::Int, const Ice::Current&);

    virtual void throwModA(Ice::Int, Ice::Int, const Ice::Current&);

    virtual void throwUndeclaredA(Ice::Int, const Ice::Current&);
    virtual void throwUndeclaredB(Ice::Int, Ice::Int, const Ice::Current&);
    virtual void throwUndeclaredC(Ice::Int, Ice::Int, Ice::Int, const Ice::Current&);
    virtual void throwLocalException(const Ice::Current&);
    virtual void throwNonIceException(const Ice::Current&);
    virtual void throwAssertException(const Ice::Current&);
    virtual Ice::ByteSeq throwMemoryLimitException(ICE_IN(Ice::ByteSeq), const Ice::Current&);

    virtual void throwLocalExceptionIdempotent(const Ice::Current&);

    virtual void throwAfterResponse(const Ice::Current&);
    virtual void throwAfterException(const Ice::Current&);
};

#endif
