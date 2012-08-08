// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/ObserverHelper.h>
#include <Ice/Observer.h>
#include <Ice/Reference.h>
#include <Ice/Instance.h>
#include <Ice/Proxy.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void
ObserverHelper::attach(const Ice::Instrumentation::ObserverPtr& observer)
{
    observer->attach();
}

void
ObserverHelper::detach(const Ice::Instrumentation::ObserverPtr& observer)
{
    observer->attach();
}

void
ObserverHelper::failed(const Ice::Instrumentation::ObserverPtr& observer, const std::string& exceptionName)
{
    observer->failed(exceptionName);
}

InvocationObserver::InvocationObserver(IceProxy::Ice::Object* proxy, const string& operation, const Context* context)
{
    if(proxy->__reference()->getInstance()->initializationData().observerResolver)
    {
        attach(proxy, operation, context);
    }
}

InvocationObserver::InvocationObserver()
{
}

void
InvocationObserver::attach(IceProxy::Ice::Object* proxy, const string& operation, const Context* context)
{
    const Ice::Instrumentation::ObserverResolverPtr& resolver =
        proxy->__reference()->getInstance()->initializationData().observerResolver;
    if(resolver)
    {
        if(context)
        {
            ObserverHelperT::attach(resolver->getInvocationObserverWithContext(proxy, operation, *context));
        }
        else
        {
            ObserverHelperT::attach(resolver->getInvocationObserver(proxy, operation));
        }
    }
}

void
InvocationObserver::retryImpl()
{
    _observer->retried();
}
