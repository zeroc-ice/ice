// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_I_H
#define TEST_I_H

#include <TestAMD.h>

class MetricsI : public Test::Metrics
{
public:

#ifdef ICE_CPP11_MAPPING
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
#else
    virtual void op_async(const Test::AMD_Metrics_opPtr&, const Ice::Current&);

    virtual void fail_async(const Test::AMD_Metrics_failPtr&, const Ice::Current&);

    virtual void opWithUserException_async(const Test::AMD_Metrics_opWithUserExceptionPtr&, const Ice::Current&);

    virtual void opWithRequestFailedException_async(const Test::AMD_Metrics_opWithRequestFailedExceptionPtr&, 
                                                    const Ice::Current&);

    virtual void opWithLocalException_async(const Test::AMD_Metrics_opWithLocalExceptionPtr&, const Ice::Current&);

    virtual void opWithUnknownException_async(const Test::AMD_Metrics_opWithUnknownExceptionPtr&, const Ice::Current&);

    virtual void opByteS_async(const Test::AMD_Metrics_opByteSPtr&, const Test::ByteSeq&, const Ice::Current&);
#endif

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
