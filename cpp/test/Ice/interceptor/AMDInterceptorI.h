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

    IceInternal::UniquePtr<IceUtil::Exception> _exception;

    IceUtil::Mutex _mutex;
};
ICE_DEFINE_PTR(AMDInterceptorIPtr, AMDInterceptorI);

#endif
