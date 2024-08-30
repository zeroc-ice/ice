//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Test.h"
#include "TestHelper.h"

using namespace std;
using namespace Ice;
using namespace Test;

// Verifies max dispatches is implemented correctly.
void
testMaxDispatches(const TestIntfPrx& p, int maxCount)
{
    cout << "testing max dispatches max = " << maxCount << "... " << flush;
    std::vector<std::future<void>> futureList;

    for (int i = 0; i < maxCount + 20; ++i)
    {
        futureList.push_back(p->opAsync());
    }

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

    string proxyStringMax10 = "test: " + helper->getTestEndpoint(1);
    TestIntfPrx pMax10{communicator, proxyStringMax10};

    string proxyStringMax1 = "test: " + helper->getTestEndpoint(2);
    TestIntfPrx pMax1{communicator, proxyStringMax1};

    string proxyStringSerialize = "test: " + helper->getTestEndpoint(3);
    TestIntfPrx pSerialize{communicator, proxyStringSerialize};

    testMaxDispatches(p, 100);
    testMaxDispatches(pMax10, 10);
    testMaxDispatches(pMax1, 1);

    // Serialize does not limit dispatches with "true" AMD.
    testMaxDispatches(pSerialize, 100);

    p->shutdown();
}
