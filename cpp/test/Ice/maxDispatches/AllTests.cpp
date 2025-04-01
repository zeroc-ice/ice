// Copyright (c) ZeroC, Inc.

#include "Test.h"
#include "TestHelper.h"

#include <chrono>
#include <thread>

using namespace std;
using namespace Ice;
using namespace Test;

// Verifies max dispatches is implemented correctly.
void
testMaxDispatches(const TestIntfPrx& p, const ResponderPrx& responder, int maxCount)
{
    cout << "testing max dispatches max = " << maxCount << "... " << flush;

    // Make sure we start fresh
    responder->stop();
    test(responder->pendingResponseCount() == 0);
    p->resetMaxConcurrentDispatches();

    std::vector<std::future<void>> futureList;

    futureList.reserve(static_cast<size_t>(maxCount) + 20);
    for (int i = 0; i < maxCount + 20; ++i)
    {
        futureList.push_back(p->opAsync());
    }

    // Wait until the responder gets at least maxCount responses.
    while (responder->pendingResponseCount() < maxCount)
    {
        this_thread::sleep_for(10ms);
    }

    responder->start();
    for (auto& future : futureList)
    {
        future.get();
    }

    int maxConcurrentDispatches = p->resetMaxConcurrentDispatches();
    test(maxConcurrentDispatches == maxCount);
    cout << "ok" << endl;
}

void
allTests(TestHelper* helper)
{
    CommunicatorPtr communicator = helper->communicator();
    string proxyString = "test: " + helper->getTestEndpoint();
    TestIntfPrx p(communicator, proxyString);

    string responderString = "responder: " + helper->getTestEndpoint(1);
    ResponderPrx responder(communicator, responderString);

    string proxyStringMax10 = "test: " + helper->getTestEndpoint(2);
    TestIntfPrx pMax10{communicator, proxyStringMax10};

    string proxyStringMax1 = "test: " + helper->getTestEndpoint(3);
    TestIntfPrx pMax1{communicator, proxyStringMax1};

    string proxyStringSerialize = "test: " + helper->getTestEndpoint(4);
    TestIntfPrx pSerialize{communicator, proxyStringSerialize};

    testMaxDispatches(p, responder, 100);
    testMaxDispatches(pMax10, responder, 10);
    testMaxDispatches(pMax1, responder, 1);

    // Serialize does not limit dispatches with "true" AMD.
    testMaxDispatches(pSerialize, responder, 100);

    p->shutdown();
}
