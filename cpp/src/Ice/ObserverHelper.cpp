// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/ObserverHelper.h>
#include <Ice/Reference.h>
#include <Ice/Instance.h>
#include <Ice/Proxy.h>

using namespace std;
using namespace Ice;
using namespace Ice::Instrumentation;

IceInternal::InvocationObserver::InvocationObserver(IceProxy::Ice::Object* proxy, const string& op, const Context* ctx)
{
    const CommunicatorObserverPtr& obsv = proxy->__reference()->getInstance()->initializationData().observer;
    if(!obsv)
    {
        return;
    }

    if(ctx)
    {
        attach(obsv->getInvocationObserverWithContext(proxy, op, *ctx));
    }
    else
    {
        attach(obsv->getInvocationObserver(proxy, op));
    }
}

IceInternal::InvocationObserver::InvocationObserver(IceInternal::Instance* instance, const string& op)
{
    const CommunicatorObserverPtr& obsv = instance->initializationData().observer;
    if(!obsv)
    {
        return;
    }

    attach(obsv->getInvocationObserver(0, op));
}

void
IceInternal::InvocationObserver::attach(IceProxy::Ice::Object* proxy, const string& op, const Context* ctx)
{
    const CommunicatorObserverPtr& obsv = proxy->__reference()->getInstance()->initializationData().observer;
    if(!obsv)
    {
        return;
    }

    if(ctx)
    {
        attach(obsv->getInvocationObserverWithContext(proxy, op, *ctx));
    }
    else
    {
        attach(obsv->getInvocationObserver(proxy, op));
    }
}

void
IceInternal::InvocationObserver::attach(IceInternal::Instance* instance, const string& op)
{
    const CommunicatorObserverPtr& obsv = instance->initializationData().observer;
    if(!obsv)
    {
        return;
    }

    attach(obsv->getInvocationObserver(0, op));
}
