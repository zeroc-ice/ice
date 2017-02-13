// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;

namespace
{

class BatchRequestInterceptorI : public Ice::BatchRequestInterceptor
{
public:

    BatchRequestInterceptorI() : _enabled(false), _count(0), _size(0), _lastRequestSize(0)
    {
    }

    virtual void
    enqueue(const Ice::BatchRequest& request, Ice::Int count, Ice::Int size)
    {
        test(request.getOperation() == "opByteSOneway" || request.getOperation() == "ice_ping");
        test(request.getProxy()->ice_isBatchOneway());

        if(count > 0)
        {
            test(_lastRequestSize + _size == size);
        }
        _count = count;
        _size = size;

        if(_size + request.getSize() > 25000)
        {
            request.getProxy()->begin_ice_flushBatchRequests();
            _size = 18; // header
        }

        if(_enabled)
        {
            _lastRequestSize = request.getSize();
            ++_count;
            request.enqueue();
        }
    }

    void
    enqueue(bool enabled)
    {
        _enabled = enabled;
    }

    int
    count()
    {
        return _count;
    }

private:

    bool _enabled;
    int _count;
    int _size;
    int _lastRequestSize;
};
typedef IceUtil::Handle<BatchRequestInterceptorI> BatchRequestInterceptorIPtr;

}

void
batchOneways(const Test::MyClassPrx& p)
{
    const Test::ByteS bs1(10  * 1024);

    Test::MyClassPrx batch = Test::MyClassPrx::uncheckedCast(p->ice_batchOneway());

    batch->ice_flushBatchRequests(); // Empty flush

    int i;
    p->opByteSOnewayCallCount(); // Reset the call count
    for(i = 0 ; i < 30 ; ++i)
    {
        try
        {
            batch->opByteSOneway(bs1);
        }
        catch(const Ice::LocalException&)
        {
            test(false);
        }
    }

    int count = 0;
    while(count < 27) // 3 * 9 requests auto-flushed.
    {
        count += p->opByteSOnewayCallCount();
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(10));
    }

    if(batch->ice_getConnection())
    {
        Test::MyClassPrx batch1 = Test::MyClassPrx::uncheckedCast(p->ice_batchOneway());
        Test::MyClassPrx batch2 = Test::MyClassPrx::uncheckedCast(p->ice_batchOneway());

        batch1->ice_ping();
        batch2->ice_ping();
        batch1->ice_flushBatchRequests();
        batch1->ice_getConnection()->close(false);
        batch1->ice_ping();
        batch2->ice_ping();

        batch1->ice_getConnection();
        batch2->ice_getConnection();

        batch1->ice_ping();
        batch1->ice_getConnection()->close(false);
        batch1->ice_ping();
        batch2->ice_ping();
    }

    Ice::Identity identity;
    identity.name = "invalid";
    Ice::ObjectPrx batch3 = batch->ice_identity(identity);
    batch3->ice_ping();
    batch3->ice_flushBatchRequests();

    // Make sure that a bogus batch request doesn't cause troubles to other ones.
    batch3->ice_ping();
    batch->ice_ping();
    batch->ice_flushBatchRequests();
    batch->ice_ping();

    if(batch->ice_getConnection())
    {
        Ice::InitializationData initData;
        initData.properties = p->ice_getCommunicator()->getProperties()->clone();
        BatchRequestInterceptorIPtr interceptor = new BatchRequestInterceptorI;

#ifdef ICE_CPP11
        // Ensure lambda factory method works.
        initData.batchRequestInterceptor = Ice::newBatchRequestInterceptor(
            [=](const Ice::BatchRequest& request, int count, int size)
            {
                interceptor->enqueue(request, count, size);
            });
#else
        initData.batchRequestInterceptor = interceptor;
#endif
        Ice::CommunicatorPtr ic = Ice::initialize(initData);

        Test::MyClassPrx batch =
            Test::MyClassPrx::uncheckedCast(ic->stringToProxy(p->ice_toString()))->ice_batchOneway();

        test(interceptor->count() == 0);
        batch->ice_ping();
        batch->ice_ping();
        batch->ice_ping();
        test(interceptor->count() == 0);

        interceptor->enqueue(true);
        batch->ice_ping();
        batch->ice_ping();
        batch->ice_ping();
        test(interceptor->count() == 3);

        batch->ice_flushBatchRequests();
        batch->ice_ping();
        test(interceptor->count() == 1);

        batch->opByteSOneway(bs1);
        test(interceptor->count() == 2);
        batch->opByteSOneway(bs1);
        test(interceptor->count() == 3);

        batch->opByteSOneway(bs1); // This should trigger the flush
        batch->ice_ping();
        test(interceptor->count() == 2);

        ic->destroy();
    }

}
