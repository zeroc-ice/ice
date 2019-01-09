// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestHelper.h>

namespace
{

int nRetry = 0;
int nFailure = 0;
int nInvocation = 0;
IceUtil::Mutex* staticMutex = 0;

class Init
{
public:

    Init()
    {
        staticMutex = new IceUtil::Mutex;
    }

    ~Init()
    {
        delete staticMutex;
        staticMutex = 0;
    }
};
Init init;

class InvocationObserverI : public Ice::Instrumentation::InvocationObserver
{
public:

    virtual void
    attach()
    {
    }

    virtual void
    detach()
    {
        IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(staticMutex);
        ++nInvocation;
    }

    virtual void
    failed(const ::std::string&)
    {
        IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(staticMutex);
        ++nFailure;
    }

    virtual void
    retried()
    {
        IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(staticMutex);
        ++nRetry;
    }

    virtual void
    userException()
    {
    }

    virtual ::Ice::Instrumentation::RemoteObserverPtr
    getRemoteObserver(const ::Ice::ConnectionInfoPtr&, const ::Ice::EndpointPtr&, ::Ice::Int, ::Ice::Int)
    {
        return ICE_NULLPTR;
    }

    virtual ::Ice::Instrumentation::CollocatedObserverPtr
    getCollocatedObserver(const Ice::ObjectAdapterPtr&, ::Ice::Int, ::Ice::Int)
    {
        return ICE_NULLPTR;
    }

};

Ice::Instrumentation::InvocationObserverPtr invocationObserver = ICE_MAKE_SHARED(InvocationObserverI);

class CommunicatorObserverI : public Ice::Instrumentation::CommunicatorObserver
{
public:

    virtual Ice::Instrumentation::ObserverPtr
    getConnectionEstablishmentObserver(const Ice::EndpointPtr&, const ::std::string&)
    {
        return ICE_NULLPTR;
    }

    virtual Ice::Instrumentation::ObserverPtr
    getEndpointLookupObserver(const Ice::EndpointPtr&)
    {
        return ICE_NULLPTR;
    }

    virtual Ice::Instrumentation::ConnectionObserverPtr
    getConnectionObserver(const Ice::ConnectionInfoPtr&,
                          const Ice::EndpointPtr&,
                          Ice::Instrumentation::ConnectionState,
                          const Ice::Instrumentation::ConnectionObserverPtr&)
    {
        return ICE_NULLPTR;
    }

    virtual Ice::Instrumentation::ThreadObserverPtr
    getThreadObserver(const ::std::string&,
                      const ::std::string&,
                      Ice::Instrumentation::ThreadState,
                      const Ice::Instrumentation::ThreadObserverPtr&)
    {
        return ICE_NULLPTR;
    }

    virtual Ice::Instrumentation::InvocationObserverPtr
    getInvocationObserver(const Ice::ObjectPrxPtr&, const ::std::string&, const Ice::Context&)
    {
        return invocationObserver;
    }

    virtual Ice::Instrumentation::DispatchObserverPtr
    getDispatchObserver(const Ice::Current&, Ice::Int)
    {
        return ICE_NULLPTR;
    }

    virtual void
    setObserverUpdater(const Ice::Instrumentation::ObserverUpdaterPtr&)
    {
    }
};

Ice::Instrumentation::CommunicatorObserverPtr communicatorObserver = ICE_MAKE_SHARED(CommunicatorObserverI);

void
testEqual(int& value, int expected)
{
    if(expected < 0)
    {
        value = 0;
        return;
    }

    int retry = 0;
    while(++retry < 100)
    {
        {
            IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(staticMutex);
            if(value == expected)
            {
                break;
            }
        }
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(10));
    }
    if(value != expected)
    {
        std::cerr << "value = " << value << ", expected = " << expected << std::endl;
        test(false);
    }
    value = 0;
}

}

void initCounts()
{
    nRetry = 0;
    nFailure = 0;
    nInvocation = 0;
}

Ice::Instrumentation::CommunicatorObserverPtr
getObserver()
{
    return communicatorObserver;
}

void
testRetryCount(int expected)
{
    testEqual(nRetry, expected);
}

void
testFailureCount(int expected)
{
    testEqual(nFailure, expected);
}

void
testInvocationCount(int expected)
{
    testEqual(nInvocation, expected);
}
