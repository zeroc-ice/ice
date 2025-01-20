// Copyright (c) ZeroC, Inc.

#ifndef TEST_I_H
#define TEST_I_H

#include "Test.h"

class TestI : public Test::TestIntf
{
public:
    void requestFailedException(const Ice::Current&) override;
    void unknownUserException(const Ice::Current&) override;
    void unknownLocalException(const Ice::Current&) override;
    void unknownException(const Ice::Current&) override;
    void userException(const Ice::Current&) override;
    void localException(const Ice::Current&) override;
    void stdException(const Ice::Current&) override;
    void cppException(const Ice::Current&) override;

    void unknownExceptionWithServantException(const Ice::Current&) override;

    std::string impossibleException(bool, const Ice::Current&) override;
    std::string intfUserException(bool, const Ice::Current&) override;

    void asyncResponse(const Ice::Current&) override;
    void asyncException(const Ice::Current&) override;

    void shutdown(const Ice::Current&) override;
};

class Cookie
{
public:
    [[nodiscard]] std::string message() const;
};

#endif
