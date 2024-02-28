//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestHelper.h>

#include <thread>
#include <chrono>
#include <mutex>

using namespace std;

namespace
{

    int nRetry = 0;
    int nFailure = 0;
    int nInvocation = 0;
    mutex staticMutex;

    class InvocationObserverI : public Ice::Instrumentation::InvocationObserver
    {
    public:
        virtual void attach() {}

        virtual void detach()
        {
            lock_guard lock(staticMutex);
            ++nInvocation;
        }

        virtual void failed(const ::std::string&)
        {
            lock_guard lock(staticMutex);
            ++nFailure;
        }

        virtual void retried()
        {
            lock_guard lock(staticMutex);
            ++nRetry;
        }

        virtual void userException() {}

        virtual ::Ice::Instrumentation::RemoteObserverPtr
        getRemoteObserver(const ::Ice::ConnectionInfoPtr&, const ::Ice::EndpointPtr&, ::int32_t, ::int32_t)
        {
            return nullptr;
        }

        virtual ::Ice::Instrumentation::CollocatedObserverPtr
        getCollocatedObserver(const Ice::ObjectAdapterPtr&, ::int32_t, ::int32_t)
        {
            return nullptr;
        }
    };

    Ice::Instrumentation::InvocationObserverPtr invocationObserver = make_shared<InvocationObserverI>();

    class CommunicatorObserverI : public Ice::Instrumentation::CommunicatorObserver
    {
    public:
        virtual Ice::Instrumentation::ObserverPtr
        getConnectionEstablishmentObserver(const Ice::EndpointPtr&, const ::std::string&)
        {
            return nullptr;
        }

        virtual Ice::Instrumentation::ObserverPtr getEndpointLookupObserver(const Ice::EndpointPtr&) { return nullptr; }

        virtual Ice::Instrumentation::ConnectionObserverPtr getConnectionObserver(
            const Ice::ConnectionInfoPtr&,
            const Ice::EndpointPtr&,
            Ice::Instrumentation::ConnectionState,
            const Ice::Instrumentation::ConnectionObserverPtr&)
        {
            return nullptr;
        }

        virtual Ice::Instrumentation::ThreadObserverPtr getThreadObserver(
            const ::std::string&,
            const ::std::string&,
            Ice::Instrumentation::ThreadState,
            const Ice::Instrumentation::ThreadObserverPtr&)
        {
            return nullptr;
        }

        virtual Ice::Instrumentation::InvocationObserverPtr
        getInvocationObserver(const Ice::ObjectPrxPtr&, const ::std::string&, const Ice::Context&)
        {
            return invocationObserver;
        }

        virtual Ice::Instrumentation::DispatchObserverPtr getDispatchObserver(const Ice::Current&, int32_t)
        {
            return nullptr;
        }

        virtual void setObserverUpdater(const Ice::Instrumentation::ObserverUpdaterPtr&) {}
    };

    Ice::Instrumentation::CommunicatorObserverPtr communicatorObserver = make_shared<CommunicatorObserverI>();

    void testEqual(int& value, int expected)
    {
        if (expected < 0)
        {
            value = 0;
            return;
        }

        int retry = 0;
        while (++retry < 100)
        {
            {
                lock_guard lock(staticMutex);
                if (value == expected)
                {
                    break;
                }
            }
            this_thread::sleep_for(chrono::milliseconds(10));
        }
        if (value != expected)
        {
            std::cerr << "value = " << value << ", expected = " << expected << std::endl;
            test(false);
        }
        value = 0;
    }

}

void
initCounts()
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
