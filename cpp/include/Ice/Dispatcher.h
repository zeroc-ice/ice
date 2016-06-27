// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_DISPATCHER_H
#define ICE_DISPATCHER_H

#include <IceUtil/Config.h>
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
