// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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

/**
 * Base class for a dispatch interceptor, which is a servant that dispatches requests
 * to another servant. A subclass must implement the dispatch method. A dispatch interceptor
 * can be registered with an object adapter just like any other servant.
 * \headerfile Ice/Ice.h
 */
class ICE_API DispatchInterceptor : public virtual Object
{
public:

    /**
     * Called by the Ice run time when a new request needs to be dispatched. The implementation
     * must eventually call ice_dispatch on the delegate servant and pass the given request object.
     * @param req An opaque object representing the request to be dispatched.
     * @return True if the request was dispatched synchronously, or false if the request was
     * dispatched asynchronously.
     */
    virtual bool dispatch(Request& req) = 0;

    /// \cond INTERNAL
    virtual bool _iceDispatch(IceInternal::Incoming&, const Current&);
    /// \endcond
};

ICE_DEFINE_PTR(DispatchInterceptorPtr, DispatchInterceptor);

}

#endif
