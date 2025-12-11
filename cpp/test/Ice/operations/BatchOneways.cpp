// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "Test.h"
#include "TestHelper.h"

#include <chrono>
#include <thread>

using namespace std;
using namespace Test;
using namespace Ice;

namespace
{
    class BatchRequestInterceptorI final
    {
    public:
        BatchRequestInterceptorI() = default;

        virtual void enqueue(const BatchRequest& request, int32_t count, int32_t size)
        {
            test(request.getOperation() == "opByteSOneway" || request.getOperation() == "ice_ping");
            test(request.getProxy()->ice_isBatchOneway());

            if (count > 0)
            {
                test(_lastRequestSize + _size == size);
            }
            _count = count;
            _size = size;

            if (_size + request.getSize() > 25000)
            {
                request.getProxy()->ice_flushBatchRequestsAsync(nullptr);
                _size = 18; // header
            }

            if (_enabled)
            {
                _lastRequestSize = request.getSize();
                ++_count;
                request.enqueue();
            }
        }

        void enqueue(bool enabled) { _enabled = enabled; }

        int count() { return _count; }

    private:
        bool _enabled{false};
        int _count{0};
        int _size{0};
        int _lastRequestSize{0};
    };
    using BatchRequestInterceptorIPtr = std::shared_ptr<BatchRequestInterceptorI>;
}

void
batchOneways(const MyClassPrx& p)
{
    const ByteS bs1(10 * 1024);

    MyClassPrx batch = p->ice_batchOneway();

    batch->ice_flushBatchRequests(); // Empty flush
    if (batch->ice_getConnection())
    {
        batch->ice_getConnection()->flushBatchRequests(CompressBatch::BasedOnProxy);
    }
    batch->ice_getCommunicator()->flushBatchRequests(CompressBatch::BasedOnProxy);

    int i;
    p->opByteSOnewayCallCount(); // Reset the call count
    for (i = 0; i < 30; ++i)
    {
        try
        {
            batch->opByteSOneway(bs1);
        }
        catch (const LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
    }

    int count = 0;
    while (count < 27) // 3 * 9 requests auto-flushed.
    {
        count += p->opByteSOnewayCallCount();
        this_thread::sleep_for(chrono::milliseconds(10));
    }

    const string protocol{p->ice_getCommunicator()->getProperties()->getIceProperty("Ice.Default.Protocol")};
    bool bluetooth = protocol == "bt" || protocol == "bts";
    if (batch->ice_getConnection() && !bluetooth)
    {
        MyClassPrx batch1 = p->ice_batchOneway();
        MyClassPrx batch2 = p->ice_batchOneway();

        batch1->ice_ping();
        batch2->ice_ping();
        batch1->ice_flushBatchRequests();
        batch1->ice_getConnection()->close().get();
        batch1->ice_ping();
        batch2->ice_ping();

        batch1->ice_getConnection();
        batch2->ice_getConnection();

        batch1->ice_ping();
        batch1->ice_getConnection()->close().get();
        batch1->ice_ping();
        batch2->ice_ping();
    }

    Identity identity;
    identity.name = "invalid";
    {
        ObjectPrx batch3 = batch->ice_identity(identity);
        batch3->ice_ping();
        batch3->ice_flushBatchRequests();
        // Make sure that a bogus batch request doesn't cause troubles to other ones.
        batch3->ice_ping();
        batch->ice_ping();
        batch->ice_flushBatchRequests();
        batch->ice_ping();
    }

    if (batch->ice_getConnection() && !bluetooth)
    {
        InitializationData initData;
        initData.properties = p->ice_getCommunicator()->getProperties()->clone();
        BatchRequestInterceptorIPtr interceptor = std::make_shared<BatchRequestInterceptorI>();

        initData.batchRequestInterceptor = [=](const BatchRequest& request, int countP, int size)
        { interceptor->enqueue(request, countP, size); };

        installTransport(initData);

        CommunicatorPtr ic = initialize(initData);

        auto batch4 = MyClassPrx(ic, p->ice_toString())->ice_batchOneway();

        test(interceptor->count() == 0);
        batch4->ice_ping();
        batch4->ice_ping();
        batch4->ice_ping();
        test(interceptor->count() == 0);

        interceptor->enqueue(true);
        batch4->ice_ping();
        batch4->ice_ping();
        batch4->ice_ping();
        test(interceptor->count() == 3);

        batch4->ice_flushBatchRequests();
        batch4->ice_ping();
        test(interceptor->count() == 1);

        batch4->opByteSOneway(bs1);
        test(interceptor->count() == 2);
        batch4->opByteSOneway(bs1);
        test(interceptor->count() == 3);

        batch4->opByteSOneway(bs1); // This should trigger the flush
        batch4->ice_ping();
        test(interceptor->count() == 2);

        ic->destroy();
    }

    bool supportsCompress = true;
    try
    {
        supportsCompress = p->supportsCompress();
    }
    catch (const OperationNotExistException&)
    {
    }

    if (supportsCompress && batch->ice_getConnection() &&
        p->ice_getCommunicator()->getProperties()->getIceProperty("Ice.Override.Compress") == "")
    {
        MyClassPrx prx = batch->ice_fixed(batch->ice_getConnection())->ice_batchOneway();

        MyClassPrx batch1 = prx->ice_compress(false);
        MyClassPrx batch2 = prx->ice_compress(true);
        auto batch3 = prx->ice_identity<MyClassPrx>(identity);

        batch1->opByteSOneway(bs1);
        batch1->opByteSOneway(bs1);
        batch1->opByteSOneway(bs1);
        batch1->ice_getConnection()->flushBatchRequests(CompressBatch::Yes);

        batch2->opByteSOneway(bs1);
        batch2->opByteSOneway(bs1);
        batch2->opByteSOneway(bs1);
        batch1->ice_getConnection()->flushBatchRequests(CompressBatch::No);

        batch1->opByteSOneway(bs1);
        batch1->opByteSOneway(bs1);
        batch1->opByteSOneway(bs1);
        batch1->ice_getConnection()->flushBatchRequests(CompressBatch::BasedOnProxy);

        batch1->opByteSOneway(bs1);
        batch2->opByteSOneway(bs1);
        batch1->opByteSOneway(bs1);
        batch1->ice_getConnection()->flushBatchRequests(CompressBatch::BasedOnProxy);

        batch1->opByteSOneway(bs1);
        batch3->opByteSOneway(bs1);
        batch1->opByteSOneway(bs1);
        batch1->ice_getConnection()->flushBatchRequests(CompressBatch::BasedOnProxy);
    }
}
