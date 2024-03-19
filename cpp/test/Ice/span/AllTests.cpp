//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Ice/Ice.h"
#include "TestHelper.h"
#include "Test.h"

#include <iterator>

using namespace std;
using namespace Ice;
using namespace Test;

TestIntfPrx
allTests(TestHelper* helper)
{
    CommunicatorPtr communicator = helper->communicator();
    TestIntfPrx prx(communicator, "test:" + helper->getTestEndpoint());

    cout << "testing span... " << flush;

    {
        vector<byte> v{std::byte{0}, std::byte{1}, std::byte{2}, std::byte{3}, std::byte{4}, std::byte{5}};

        auto dataIn = span<byte>{v}.subspan(1, 3);
        vector<byte> dataOut;
        auto r = prx->opByteSpan(dataIn, dataOut);
        vector<byte> dataInVec{dataIn.begin(), dataIn.end()};
        test(r == dataOut);
        test(r == dataInVec);
    }
    cout << "ok" << endl;
    return prx;
}
