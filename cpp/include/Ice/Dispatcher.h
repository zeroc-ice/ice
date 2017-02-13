// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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

class ICE_API DispatcherCall : public virtual IceUtil::Shared
{
public:

    virtual ~DispatcherCall();

    virtual void run() = 0;
};

typedef IceUtil::Handle<DispatcherCall> DispatcherCallPtr;

class ICE_API Dispatcher : public virtual IceUtil::Shared
{
public:

    virtual ~Dispatcher();

    virtual void dispatch(const DispatcherCallPtr&, const ConnectionPtr&) = 0;
};

typedef IceUtil::Handle<Dispatcher> DispatcherPtr;

}

#endif
#endif
