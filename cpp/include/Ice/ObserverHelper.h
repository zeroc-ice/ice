// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_OBSERVERHELPER_H
#define ICE_OBSERVERHELPER_H

#include <Ice/Instrumentation.h>
#include <Ice/ProxyF.h>
#include <Ice/InstanceF.h>

namespace IceInternal
{

template<typename T = Ice::Instrumentation::Observer> class ObserverHelperT
{
public:

    typedef IceInternal::Handle<T> TPtr;

    ObserverHelperT()
    {
    }

    ~ObserverHelperT()
    {
        if(_observer)
        {
            _observer->detach();
        }
    }

    operator bool() const
    {
        return _observer;
    }

    T* operator->() const
    {
        return _observer.get();
    }

    void
    attach(const TPtr& o)
    {
        //
        // Don't detach the existing observer. The observer is being
        // replaced and the observed object is still being observed!
        //
        // if(_observer)
        // {
        //     _observer->detach();
        // }
        _observer = o;
        if(_observer)
        {
            _observer->attach();
        }
    }

    T* get() const
    {
        return _observer.get();
    }

    void adopt(ObserverHelperT& other)
    {
        _observer = other._observer;
        other._observer = 0;
    }

    void detach()
    {
        if(_observer)
        {
            _observer->detach();
            _observer = 0;
        }
    }

    void failed(const std::string& reason)
    {
        if(_observer)
        {
            _observer->failed(reason);
        }
    }

protected:

    TPtr _observer;
};

class ICE_API DispatchObserver : public ObserverHelperT<Ice::Instrumentation::DispatchObserver>
{
public:
    
    void userException()
    {
        if(_observer)
        {
            _observer->userException();
        }
    }

    void reply(Ice::Int size)
    {
        if(_observer)
        {
            _observer->reply(size);
        }
    }
};

class ICE_API InvocationObserver : public ObserverHelperT<Ice::Instrumentation::InvocationObserver>
{
public:

    InvocationObserver(IceProxy::Ice::Object*, const std::string&, const Ice::Context*);
    InvocationObserver(Instance*, const std::string&);
    InvocationObserver()
    {
    }

    void attach(IceProxy::Ice::Object*, const std::string&, const Ice::Context*);
    void attach(Instance*, const std::string&);

    void retried()
    {
        if(_observer)
        {
            _observer->retried();
        }
    }

    ::Ice::Instrumentation::ChildInvocationObserverPtr
    getRemoteObserver(const Ice::ConnectionInfoPtr& con, const Ice::EndpointPtr& endpt, int requestId, int size)
    {
        if(_observer)
        {
            return _observer->getRemoteObserver(con, endpt, requestId, size);
        }
        return 0;
    }

    ::Ice::Instrumentation::ChildInvocationObserverPtr
    getCollocatedObserver(const Ice::ObjectAdapterPtr& adapter, int requestId, int size)
    {
        if(_observer)
        {
            return _observer->getCollocatedObserver(adapter, requestId, size);
        }
        return 0;
    }

    void
    userException()
    {
        if(_observer)
        {
            _observer->userException();
        }
    }

private:

    using ObserverHelperT<Ice::Instrumentation::InvocationObserver>::attach;
};

}

#endif
