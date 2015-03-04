// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
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
#ifdef ICE_CPP11
#   include <functional>
#endif

namespace Ice
{

class ICE_API DispatcherCall : virtual public IceUtil::Shared
{
public:

    virtual ~DispatcherCall() { }

    virtual void run() = 0;
};

typedef IceUtil::Handle<DispatcherCall> DispatcherCallPtr;

class ICE_API Dispatcher : virtual public IceUtil::Shared
{
public:

    virtual void dispatch(const DispatcherCallPtr&, const ConnectionPtr&) = 0;
};

typedef IceUtil::Handle<Dispatcher> DispatcherPtr;

}

#ifdef ICE_CPP11
namespace IceInternal
{
class ICE_API Cpp11Dispatcher : public ::Ice::Dispatcher
{
public:
    
    Cpp11Dispatcher(const ::std::function<void (const ::Ice::DispatcherCallPtr&, const ::Ice::ConnectionPtr)>& cb) :
        _cb(cb)
    {
    }
    
    virtual void dispatch(const ::Ice::DispatcherCallPtr& call, const ::Ice::ConnectionPtr& conn);
    
private:
    
    const ::std::function<void (const ::Ice::DispatcherCallPtr&, const ::Ice::ConnectionPtr)> _cb;
};

}

namespace Ice
{

inline DispatcherPtr
newDispatcher(const ::std::function<void (const ::Ice::DispatcherCallPtr&, const ::Ice::ConnectionPtr)>& cb)
{
    return new ::IceInternal::Cpp11Dispatcher(cb);
}

}

#endif
#endif
