// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

#include <Ice/Object.h>

namespace Ice
{

class ICE_API DispatchInterceptor : public virtual Object
{
public:

    virtual DispatchStatus
    dispatch(Request&) = 0;

    virtual DispatchStatus
    __dispatch(IceInternal::Incoming&, const Current&);
    
    virtual DispatchStatus
    __collocDispatch(IceInternal::Direct&);
};

typedef IceInternal::Handle<DispatchInterceptor> DispatchInterceptorPtr;

}
