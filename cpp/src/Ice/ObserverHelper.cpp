// Copyright (c) ZeroC, Inc.

#include "Ice/ObserverHelper.h"
#include "Ice/Proxy.h"
#include "Instance.h"
#include "Reference.h"

using namespace std;
using namespace Ice;
using namespace Ice::Instrumentation;

IceInternal::InvocationObserver::InvocationObserver(const Ice::ObjectPrx& proxy, string_view op, const Context& ctx)
{
    const CommunicatorObserverPtr& obsv = proxy->_getReference()->getInstance()->initializationData().observer;
    if (!obsv)
    {
        return;
    }
    attach(obsv->getInvocationObserver(proxy, op, ctx));
}

IceInternal::InvocationObserver::InvocationObserver(IceInternal::Instance* instance, string_view op)
{
    const CommunicatorObserverPtr& obsv = instance->initializationData().observer;
    if (!obsv)
    {
        return;
    }

    attach(obsv->getInvocationObserver(nullopt, op, noExplicitContext));
}

void
IceInternal::InvocationObserver::attach(const Ice::ObjectPrx& proxy, string_view op, const Context& ctx)
{
    const CommunicatorObserverPtr& obsv = proxy->_getReference()->getInstance()->initializationData().observer;
    if (!obsv)
    {
        return;
    }
    attach(obsv->getInvocationObserver(proxy, op, ctx));
}

void
IceInternal::InvocationObserver::attach(IceInternal::Instance* instance, string_view op)
{
    const CommunicatorObserverPtr& obsv = instance->initializationData().observer;
    if (!obsv)
    {
        return;
    }

    attach(obsv->getInvocationObserver(nullopt, op, Ice::noExplicitContext));
}

std::string
IceInternal::getExceptionId(std::exception_ptr eptr)
{
    try
    {
        std::rethrow_exception(eptr);
    }
    catch (const Ice::Exception& ex)
    {
        return ex.ice_id();
    }
    catch (const std::exception& ex)
    {
        return ex.what();
    }
    catch (...)
    {
        return "unknown";
    }
}
