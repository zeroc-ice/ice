//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/ObserverHelper.h>
#include <Ice/Reference.h>
#include <Ice/Instance.h>
#include <Ice/Proxy.h>

using namespace std;
using namespace Ice;
using namespace Ice::Instrumentation;

IceInternal::InvocationObserver::InvocationObserver(const Ice::ObjectPrxPtr& proxy, const string& op, const Context& ctx)
{
    const CommunicatorObserverPtr& obsv = proxy->_getReference()->getInstance()->initializationData().observer;
    if(!obsv)
    {
        return;
    }
    attach(obsv->getInvocationObserver(proxy, op, ctx));
}

IceInternal::InvocationObserver::InvocationObserver(IceInternal::Instance* instance, const string& op)
{
    const CommunicatorObserverPtr& obsv = instance->initializationData().observer;
    if(!obsv)
    {
        return;
    }

    attach(obsv->getInvocationObserver(0, op, noExplicitContext));
}

void
IceInternal::InvocationObserver::attach(const Ice::ObjectPrxPtr& proxy, const string& op, const Context& ctx)
{
    const CommunicatorObserverPtr& obsv = proxy->_getReference()->getInstance()->initializationData().observer;
    if(!obsv)
    {
        return;
    }
    attach(obsv->getInvocationObserver(proxy, op, ctx));
}

void
IceInternal::InvocationObserver::attach(IceInternal::Instance* instance, const string& op)
{
    const CommunicatorObserverPtr& obsv = instance->initializationData().observer;
    if(!obsv)
    {
        return;
    }

    attach(obsv->getInvocationObserver(0, op, Ice::noExplicitContext));
}
