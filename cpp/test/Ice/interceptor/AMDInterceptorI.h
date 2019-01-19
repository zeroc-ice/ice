//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef AMD_INTERCEPTOR_I_H
#define AMD_INTERCEPTOR_I_H

#include <InterceptorI.h>
#include <IceUtil/IceUtil.h>
#include <Ice/UniquePtr.h>

class AMDInterceptorI : public InterceptorI
{
public:

    AMDInterceptorI(const Ice::ObjectPtr&);

    virtual bool dispatch(Ice::Request&);

    virtual void clear();

    IceUtil::Exception* getException() const;
    void setException(const IceUtil::Exception&);

private:

#ifndef ICE_CPP11_MAPPING
    Ice::DispatchInterceptorAsyncCallbackPtr _defaultCb;
#endif
    IceInternal::UniquePtr<IceUtil::Exception> _exception;

    IceUtil::Mutex _mutex;
};
ICE_DEFINE_PTR(AMDInterceptorIPtr, AMDInterceptorI);

#ifndef ICE_CPP11_MAPPING
class DispatchInterceptorAsyncCallbackI : public Ice::DispatchInterceptorAsyncCallback
{
public:

    DispatchInterceptorAsyncCallbackI(AMDInterceptorI&);

    virtual bool response();
    virtual bool exception(const std::exception&);
    virtual bool exception();

private:
    AMDInterceptorI& _interceptor;
};
#endif

#endif
