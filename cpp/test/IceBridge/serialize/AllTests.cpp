//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Ice/Ice.h"
#include "Test.h"
#include "TestHelper.h"

using namespace std;

void
allTests(Test::TestHelper* helper)
{
    auto communicator = helper->communicator();
    Test::MyClassPrx cl(communicator, "test:" + helper->getTestEndpoint(1));
    cl->ice_ping();

    cout << "testing ordering... " << flush;
    {
        // Make sure ordering is preserved on each connection.
        int counter = 0;
        for (int i = 0; i < 10; ++i)
        {
            auto p = cl->ice_connectionId(to_string(i));
            for (int j = 0; j < 20; ++j)
            {
                p->incCounterAsync(++counter, nullptr);
            }
            cl->waitCounter(counter);
        }
        for (int i = 0; i < 10; ++i)
        {
            auto p = cl->ice_connectionId(to_string(i))->ice_oneway();
            for (int j = 0; j < 20; ++j)
            {
                p->incCounterAsync(++counter, nullptr);
            }
            cl->waitCounter(counter);
        }
    }
    cout << "ok" << endl;

    cl->shutdown();
    Ice::ObjectPrx admin(communicator, "IceBridge/admin:" + helper->getTestEndpoint(2, "tcp"));
    auto process = admin->ice_facet<Ice::ProcessPrx>("Process");
    process->shutdown();
}
