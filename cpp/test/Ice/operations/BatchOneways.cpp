// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <Test.h>

using namespace std;

namespace
{

class BatchRequestInterceptorI ICE_FINAL
#ifndef ICE_CPP11_MAPPING
    : public Ice::BatchRequestInterceptor
#endif
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
#ifdef ICE_CPP11_MAPPING
            request.getProxy()->ice_flushBatchRequestsAsync();
#else
            request.getProxy()->begin_ice_flushBatchRequests();
#endif
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
ICE_DEFINE_PTR(BatchRequestInterceptorIPtr, BatchRequestInterceptorI);

}

void
batchOneways(const Test::MyClassPrxPtr& p)
{
    const Test::ByteS bs1(10  * 1024);

    Test::MyClassPrxPtr batch = ICE_UNCHECKED_CAST(Test::MyClassPrx, p->ice_batchOneway());

    batch->ice_flushBatchRequests(); // Empty flush
    if(batch->ice_getConnection())
    {
        batch->ice_getConnection()->flushBatchRequests(Ice::ICE_SCOPED_ENUM(CompressBatch, BasedOnProxy));
    }
    batch->ice_getCommunicator()->flushBatchRequests(Ice::ICE_SCOPED_ENUM(CompressBatch, BasedOnProxy));

    int i;
    p->opByteSOnewayCallCount(); // Reset the call count
    for(i = 0 ; i < 30 ; ++i)
    {
        try
        {
            batch->opByteSOneway(bs1);
        }
        catch(const Ice::LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
    }

    int count = 0;
    while(count < 27) // 3 * 9 requests auto-flushed.
    {
        count += p->opByteSOnewayCallCount();
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(10));
    }

    if(batch->ice_getConnection() &&
       p->ice_getCommunicator()->getProperties()->getProperty("Ice.Default.Protocol") != "bt")
    {
        Test::MyClassPrxPtr batch1 = ICE_UNCHECKED_CAST(Test::MyClassPrx, p->ice_batchOneway());
        Test::MyClassPrxPtr batch2 = ICE_UNCHECKED_CAST(Test::MyClassPrx, p->ice_batchOneway());

        batch1->ice_ping();
        batch2->ice_ping();
        batch1->ice_flushBatchRequests();
        batch1->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
        batch1->ice_ping();
        batch2->ice_ping();

        batch1->ice_getConnection();
        batch2->ice_getConnection();

        batch1->ice_ping();
        batch1->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
        batch1->ice_ping();
        batch2->ice_ping();
    }

    Ice::Identity identity;
    identity.name = "invalid";
    {
        Ice::ObjectPrxPtr batch3 = batch->ice_identity(identity);
        batch3->ice_ping();
        batch3->ice_flushBatchRequests();
        // Make sure that a bogus batch request doesn't cause troubles to other ones.
        batch3->ice_ping();
        batch->ice_ping();
        batch->ice_flushBatchRequests();
        batch->ice_ping();
    }

    if(batch->ice_getConnection() &&
       p->ice_getCommunicator()->getProperties()->getProperty("Ice.Default.Protocol") != "bt")
    {
        Ice::InitializationData initData;
        initData.properties = p->ice_getCommunicator()->getProperties()->clone();
        BatchRequestInterceptorIPtr interceptor = ICE_MAKE_SHARED(BatchRequestInterceptorI);

#if defined(ICE_CPP11_MAPPING)
        initData.batchRequestInterceptor = [=](const Ice::BatchRequest& request, int countP, int size)
        {
            interceptor->enqueue(request, countP, size);
        };
#else
        initData.batchRequestInterceptor = interceptor;
#endif
        Ice::CommunicatorPtr ic = Ice::initialize(initData);

        Test::MyClassPrxPtr batch4 =
            ICE_UNCHECKED_CAST(Test::MyClassPrx, ic->stringToProxy(p->ice_toString()))->ice_batchOneway();

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
    catch(const Ice::OperationNotExistException&)
    {
    }

    if(supportsCompress && batch->ice_getConnection() &&
       p->ice_getCommunicator()->getProperties()->getProperty("Ice.Override.Compress") == "")
    {
        Ice::ObjectPrxPtr prx = batch->ice_getConnection()->createProxy(batch->ice_getIdentity())->ice_batchOneway();

        Test::MyClassPrxPtr batch1 = ICE_UNCHECKED_CAST(Test::MyClassPrx, prx->ice_compress(false));
        Test::MyClassPrxPtr batch2 = ICE_UNCHECKED_CAST(Test::MyClassPrx, prx->ice_compress(true));
        Test::MyClassPrxPtr batch3 = ICE_UNCHECKED_CAST(Test::MyClassPrx, prx->ice_identity(identity));

        batch1->opByteSOneway(bs1);
        batch1->opByteSOneway(bs1);
        batch1->opByteSOneway(bs1);
        batch1->ice_getConnection()->flushBatchRequests(Ice::ICE_SCOPED_ENUM(CompressBatch, Yes));

        batch2->opByteSOneway(bs1);
        batch2->opByteSOneway(bs1);
        batch2->opByteSOneway(bs1);
        batch1->ice_getConnection()->flushBatchRequests(Ice::ICE_SCOPED_ENUM(CompressBatch, No));

        batch1->opByteSOneway(bs1);
        batch1->opByteSOneway(bs1);
        batch1->opByteSOneway(bs1);
        batch1->ice_getConnection()->flushBatchRequests(Ice::ICE_SCOPED_ENUM(CompressBatch, BasedOnProxy));

        batch1->opByteSOneway(bs1);
        batch2->opByteSOneway(bs1);
        batch1->opByteSOneway(bs1);
        batch1->ice_getConnection()->flushBatchRequests(Ice::ICE_SCOPED_ENUM(CompressBatch, BasedOnProxy));

        batch1->opByteSOneway(bs1);
        batch3->opByteSOneway(bs1);
        batch1->opByteSOneway(bs1);
        batch1->ice_getConnection()->flushBatchRequests(Ice::ICE_SCOPED_ENUM(CompressBatch, BasedOnProxy));
    }
}
