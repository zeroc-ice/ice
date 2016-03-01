// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef AMD_INTERCEPTOR_I_H
#define AMD_INTERCEPTOR_I_H

#include <InterceptorI.h>
#include <IceUtil/IceUtil.h>

class AMDInterceptorI : public InterceptorI
{
public:

    AMDInterceptorI(const Ice::ObjectPtr&);
    
    virtual Ice::DispatchStatus dispatch(Ice::Request&);
     
    virtual void clear();

    Ice::DispatchStatus getActualStatus() const;
#ifdef ICE_CPP11_MAPPING
    std::exception_ptr getException() const;
#else
    IceUtil::Exception* getException() const;
#endif

    void setActualStatus(Ice::DispatchStatus);
    void setActualStatus(const IceUtil::Exception&);
   
private:

    Ice::DispatchInterceptorAsyncCallbackPtr _defaultCb;
    Ice::DispatchStatus _actualStatus;
#ifdef ICE_CPP11_MAPPING
    std::exception_ptr _exception;
#else
    IceUtil::UniquePtr<IceUtil::Exception> _exception;
#endif
  
    IceUtil::Mutex _mutex;
};
ICE_DEFINE_PTR(AMDInterceptorIPtr, AMDInterceptorI);

class DispatchInterceptorAsyncCallbackI : public Ice::DispatchInterceptorAsyncCallback
{
public:

    DispatchInterceptorAsyncCallbackI(AMDInterceptorI&);

    virtual bool response(bool);
    virtual bool exception(const std::exception&);
    virtual bool exception();

private:
    AMDInterceptorI& _interceptor;
};

#endif
