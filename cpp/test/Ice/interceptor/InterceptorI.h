// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef INTERCEPTOR_I_H
#define INTERCEPTOR_I_H

#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>

class InterceptorI : public Ice::DispatchInterceptor
{
public:

    InterceptorI(const Ice::ObjectPtr&);

    virtual bool dispatch(Ice::Request& request);

    bool getLastStatus() const;
    const std::string& getLastOperation() const;

    virtual void clear();

protected:
    const Ice::ObjectPtr _servant;
    std::string _lastOperation;
    bool _lastStatus;
};
ICE_DEFINE_PTR(InterceptorIPtr, InterceptorI);

#endif
