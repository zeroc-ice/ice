// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TESTAMD_I_H
#define TESTAMD_I_H

#include <TestAMD.h>

class TestAMDI : public Test::TestIntf
{
public:

    virtual void requestFailedException_async(const Test::AMD_TestIntf_requestFailedExceptionPtr&, const Ice::Current&);
    virtual void unknownUserException_async(const Test::AMD_TestIntf_unknownUserExceptionPtr&, const Ice::Current&);
    virtual void unknownLocalException_async(const Test::AMD_TestIntf_unknownLocalExceptionPtr&, const Ice::Current&);
    virtual void unknownException_async(const Test::AMD_TestIntf_unknownExceptionPtr&, const Ice::Current&);
    virtual void userException_async(const Test::AMD_TestIntf_userExceptionPtr&, const Ice::Current&);
    virtual void localException_async(const Test::AMD_TestIntf_localExceptionPtr&, const Ice::Current&);
    virtual void stdException_async(const Test::AMD_TestIntf_stdExceptionPtr&, const Ice::Current&);
    virtual void cppException_async(const Test::AMD_TestIntf_cppExceptionPtr&, const Ice::Current&);

    virtual void unknownExceptionWithServantException_async(
                            const Test::AMD_TestIntf_unknownExceptionWithServantExceptionPtr&, const Ice::Current&);

    virtual void impossibleException_async(const Test::AMD_TestIntf_impossibleExceptionPtr&, bool, const Ice::Current&);
    virtual void intfUserException_async(const Test::AMD_TestIntf_intfUserExceptionPtr&, bool, const Ice::Current&);

    virtual void asyncResponse_async(const Test::AMD_TestIntf_asyncResponsePtr&, const Ice::Current&);
    virtual void asyncException_async(const Test::AMD_TestIntf_asyncExceptionPtr&, const Ice::Current&);

    virtual void shutdown_async(const Test::AMD_TestIntf_shutdownPtr&, const Ice::Current&);
};

class CookieI : public Test::Cookie
{
public:

    virtual std::string message() const;
};

#endif
