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

class ObserverHelper : public IceUtil::noncopyable
{
protected:

    void attach(const Ice::Instrumentation::ObserverPtr&);
    void detach(const Ice::Instrumentation::ObserverPtr&);
    void failed(const Ice::Instrumentation::ObserverPtr&, const std::string&);
};

template<typename T = Ice::Instrumentation::Observer> class ObserverHelperT : public ObserverHelper
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
            ObserverHelper::detach(_observer);
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
        if(_observer)
        {
            ObserverHelper::detach(_observer);
        }
        _observer = o;
        if(_observer)
        {
            ObserverHelper::attach(_observer);
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
            ObserverHelper::detach(_observer);
            _observer = 0;
        }
    }

    void failed(const std::string& reason)
    {
        if(_observer)
        {
            ObserverHelper::failed(_observer, reason);
        }
    }

protected:

    TPtr _observer;
};

class InvocationObserver : public ObserverHelperT<Ice::Instrumentation::InvocationObserver>
{
public:

    InvocationObserver(IceProxy::Ice::Object*, const std::string&, const Ice::Context*);
    InvocationObserver();

    void attach(IceProxy::Ice::Object*, const std::string&, const Ice::Context*);

    void retry()
    {
        if(_observer)
        {
            InvocationObserver::retryImpl();
        }
    }

private:

    void retryImpl();
};

}

#endif
