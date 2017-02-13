// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_DISPATCH_INTERCEPTOR_H
#define ICE_DISPATCH_INTERCEPTOR_H

#include <Ice/Object.h>

namespace Ice
{

class ICE_API DispatchInterceptor : public virtual Object
{
public:

    virtual bool dispatch(Request&) = 0;

    virtual bool _iceDispatch(IceInternal::Incoming&, const Current&);
};

ICE_DEFINE_PTR(DispatchInterceptorPtr, DispatchInterceptor);

}

#endif
