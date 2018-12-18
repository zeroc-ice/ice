// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_DISPATCHER_H
#define ICE_DISPATCHER_H

#if !defined(ICE_CPP11_MAPPING) || defined(ICE_BUILDING_SRC)
//
// Part of the C++98 mapping, and "internal" definitions when building Ice
// with the C++11 mapping
//

#include <Ice/Config.h>
#include <IceUtil/Shared.h>
#include <IceUtil/Handle.h>
#include <Ice/ConnectionF.h>

namespace Ice
{

/**
 * Encapsulates all the details of a request dispatch or AMI callback.
 * The application must eventually invoke run to dispatch the call.
 * \headerfile Ice/Ice.h
 */
class ICE_API DispatcherCall : public virtual IceUtil::Shared
{
public:

    virtual ~DispatcherCall();

    /**
     * Dispatches the call.
     */
    virtual void run() = 0;
};

typedef IceUtil::Handle<DispatcherCall> DispatcherCallPtr;

/**
 * Base class for a dispatcher. A subclass must define the dispatch method.
 * The dispatcher can be installed via InitializationData.
 * \headerfile Ice/Ice.h
 */
class ICE_API Dispatcher : public virtual IceUtil::Shared
{
public:

    virtual ~Dispatcher();

    /**
     * Called by the Ice run time when an incoming request or an AMI callback needs to
     * be dispatched. The implementation must eventually invoke run on the call object.
     * @param call An object representing the call that must be dispatched.
     * @param connection The connection object associated with the call, or nil if no
     * connection is associated with the call.
     */
    virtual void dispatch(const DispatcherCallPtr& call, const ConnectionPtr& connection) = 0;
};

typedef IceUtil::Handle<Dispatcher> DispatcherPtr;

}

#endif
#endif
