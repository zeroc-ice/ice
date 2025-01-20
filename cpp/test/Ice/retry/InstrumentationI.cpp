// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "TestHelper.h"

#include <chrono>
#include <mutex>
#include <thread>

using namespace std;
using namespace Ice;

namespace
{
    int nRetry = 0;
    int nFailure = 0;
    int nInvocation = 0;
    mutex staticMutex;

    class InvocationObserverI final : public Instrumentation::InvocationObserver
    {
    public:
        void attach() final {}

        void detach() final
        {
            lock_guard lock(staticMutex);
            ++nInvocation;
        }

        void failed(const string&) final
        {
            lock_guard lock(staticMutex);
            ++nFailure;
        }

        void retried() final
        {
            lock_guard lock(staticMutex);
            ++nRetry;
        }

        void userException() final {}

        Instrumentation::RemoteObserverPtr
        getRemoteObserver(const ConnectionInfoPtr&, const EndpointPtr&, int32_t, int32_t) final
        {
            return nullptr;
        }

        Instrumentation::CollocatedObserverPtr getCollocatedObserver(const ObjectAdapterPtr&, int32_t, int32_t) final
        {
            return nullptr;
        }
    };

    // NOLINTNEXTLINE(cert-err58-cpp)
    Instrumentation::InvocationObserverPtr invocationObserver = make_shared<InvocationObserverI>();

    class CommunicatorObserverI final : public Instrumentation::CommunicatorObserver
    {
    public:
        Instrumentation::ObserverPtr getConnectionEstablishmentObserver(const EndpointPtr&, const string&) final
        {
            return nullptr;
        }

        Instrumentation::ObserverPtr getEndpointLookupObserver(const EndpointPtr&) final { return nullptr; }

        Instrumentation::ConnectionObserverPtr getConnectionObserver(
            const ConnectionInfoPtr&,
            const EndpointPtr&,
            Instrumentation::ConnectionState,
            const Instrumentation::ConnectionObserverPtr&) final
        {
            return nullptr;
        }

        Instrumentation::ThreadObserverPtr getThreadObserver(
            const string&,
            const string&,
            Instrumentation::ThreadState,
            const Instrumentation::ThreadObserverPtr&) final
        {
            return nullptr;
        }

        Instrumentation::InvocationObserverPtr
        getInvocationObserver(const optional<ObjectPrx>&, string_view, const Context&) final
        {
            return invocationObserver;
        }

        Instrumentation::DispatchObserverPtr getDispatchObserver(const Current&, int32_t) final { return nullptr; }

        void setObserverUpdater(const Instrumentation::ObserverUpdaterPtr&) final {}
    };

    // NOLINTNEXTLINE(cert-err58-cpp)
    Instrumentation::CommunicatorObserverPtr communicatorObserver = make_shared<CommunicatorObserverI>();

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
            cerr << "value = " << value << ", expected = " << expected << endl;
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

Instrumentation::CommunicatorObserverPtr
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
