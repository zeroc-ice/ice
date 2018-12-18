// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class TestI : public Test::TestIntf
{
public:

    virtual void requestFailedException(const Ice::Current&);
    virtual void unknownUserException(const Ice::Current&);
    virtual void unknownLocalException(const Ice::Current&);
    virtual void unknownException(const Ice::Current&);
    virtual void userException(const Ice::Current&);
    virtual void localException(const Ice::Current&);
    virtual void stdException(const Ice::Current&);
    virtual void cppException(const Ice::Current&);

    virtual void unknownExceptionWithServantException(const Ice::Current&);

    virtual ::std::string impossibleException(bool, const Ice::Current&);
    virtual ::std::string intfUserException(bool, const Ice::Current&);

    virtual void asyncResponse(const Ice::Current&);
    virtual void asyncException(const Ice::Current&);

    virtual void shutdown(const Ice::Current&);
};

class CookieI : public Test::Cookie
{
public:

    virtual std::string message() const;
};

#endif
