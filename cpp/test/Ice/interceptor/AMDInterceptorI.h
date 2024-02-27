//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef AMD_INTERCEPTOR_I_H
#define AMD_INTERCEPTOR_I_H

#include <InterceptorI.h>
#include <IceUtil/IceUtil.h>

class AMDInterceptorI : public InterceptorI
{
public:
    AMDInterceptorI(const Ice::ObjectPtr&);

    virtual bool dispatch(Ice::Request&);

    virtual void clear();

    std::exception_ptr getException() const;
    void setException(std::exception_ptr);

private:
    std::exception_ptr _exception;

    mutable std::mutex _mutex;
};
using AMDInterceptorIPtr = std::shared_ptr<AMDInterceptorI>;

#endif
