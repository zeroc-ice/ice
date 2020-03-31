//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_I_H
#define TEST_I_H

#include <TestAMD.h>

class MetricsI : public Test::Metrics
{
public:

    virtual void opAsync(std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&);

    virtual void failAsync(std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&);

    virtual void opWithUserExceptionAsync(std::function<void()>, std::function<void(std::exception_ptr)>,
                                           const Ice::Current&);

    virtual void opWithRequestFailedExceptionAsync(std::function<void()>, std::function<void(std::exception_ptr)>,
                                                    const Ice::Current&);

    virtual void opWithLocalExceptionAsync(std::function<void()>, std::function<void(std::exception_ptr)>,
                                            const Ice::Current&);

    virtual void opWithUnknownExceptionAsync(std::function<void()>, std::function<void(std::exception_ptr)>,
                                              const Ice::Current&);

    virtual void opByteSAsync(Test::ByteSeq, std::function<void()>, std::function<void(std::exception_ptr)>,
                               const Ice::Current&);

    virtual Ice::ObjectPrxPtr getAdmin(const Ice::Current&);

    virtual void shutdown(const Ice::Current&);
};

class ControllerI : public Test::Controller
{
public:

    ControllerI(const Ice::ObjectAdapterPtr&);

    virtual void hold(const Ice::Current&);

    virtual void resume(const Ice::Current&);

private:

    const Ice::ObjectAdapterPtr _adapter;
};

#endif
