//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TESTAMD_I_H
#define TESTAMD_I_H

#include <TestAMD.h>

class TestAMDI : public Test::TestIntf
{
public:

    virtual void requestFailedExceptionAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&);

    virtual void unknownUserExceptionAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&);

    virtual void unknownLocalExceptionAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&);

    virtual void unknownExceptionAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&);

    virtual void userExceptionAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&);

    virtual void localExceptionAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&);

    virtual void stdExceptionAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&);

    virtual void cppExceptionAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&);

    virtual void unknownExceptionWithServantExceptionAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&);

    virtual void impossibleExceptionAsync(
        bool,
        std::function<void(const std::string&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&);

    virtual void intfUserExceptionAsync(
        bool,
        std::function<void(const std::string&)>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&);

    virtual void asyncResponseAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&);

    virtual void asyncExceptionAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&);

    virtual void shutdownAsync(
        std::function<void()>,
        std::function<void(std::exception_ptr)>,
        const Ice::Current&);
};

class Cookie

{
public:

    virtual ~Cookie();
    virtual std::string message() const;
};

#endif
