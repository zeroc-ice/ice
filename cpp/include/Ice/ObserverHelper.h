// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_OBSERVERHELPER_H
#define ICE_OBSERVERHELPER_H

#include <Ice/Observer.h>
#include <Ice/ProxyF.h>

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

class ICE_API InvocationObserver : public ObserverHelperT<Ice::Instrumentation::InvocationObserver>
{
public:

    InvocationObserver(IceProxy::Ice::Object*, const std::string&, const Ice::Context*);
    InvocationObserver()
    {
    }

    void attach(IceProxy::Ice::Object*, const std::string&, const Ice::Context*);

    void retried()
    {
        if(_observer)
        {
            _observer->retried();
        }
    }

    ::Ice::Instrumentation::ObserverPtr
    getRemoteObserver(const Ice::ConnectionPtr& con)
    {
        if(_observer)
        {
            return _observer->getRemoteObserver(con);
        }
        return 0;
    }
};

}

#endif
