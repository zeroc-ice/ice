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

InvocationObserver::InvocationObserver(IceProxy::Ice::Object* proxy, const string& operation, const Context* context)
{
    if(proxy->__reference()->getInstance()->initializationData().observer)
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
    const Ice::Instrumentation::CommunicatorObserverPtr& obsv = 
        proxy->__reference()->getInstance()->initializationData().observer;
    if(obsv)
    {
        if(context)
        {
            ObserverHelperT<Ice::Instrumentation::InvocationObserver>::attach(
                obsv->getInvocationObserverWithContext(proxy, operation, *context));
        }
        else
        {
            ObserverHelperT<Ice::Instrumentation::InvocationObserver>::attach(
                obsv->getInvocationObserver(proxy, operation));
        }
    }
}
