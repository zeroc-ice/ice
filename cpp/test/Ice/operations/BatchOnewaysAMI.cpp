// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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

class Callback : public IceUtil::Monitor<IceUtil::Mutex>, public IceUtil::Shared
{
public:

    Callback() :
        _called(false)
    {
    }

    virtual ~Callback()
    {
    }

    void check()
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        while(!_called)
        {
            wait();
        }
        _called = false;
    }

    void called()
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        assert(!_called);
        _called = true;
        notify();
    }

private:

    bool _called;
};
typedef IceUtil::Handle<Callback> CallbackPtr;

class Callback_ByteSOneway : public IceUtil::Shared
{
public:

    void response()
    {
    }

    void exception(const ::Ice::Exception&)
    {
        test(false);
    }
};

class Callback_ping : public IceUtil::Shared
{
    CallbackPtr _cb;

public:

    Callback_ping(const CallbackPtr& cb) : _cb(cb)
    {
    }

    void response()
    {
        test(false);
    }

    void exception(const ::Ice::Exception& ex)
    {
        test(dynamic_cast<const Ice::CloseConnectionException*>(&ex));
        _cb->called();
    }
};
}

void
batchOnewaysAMI(const Test::MyClassPrxPtr& p)
{
    const Test::ByteS bs1(10 * 1024);
    Test::MyClassPrxPtr batch = ICE_UNCHECKED_CAST(Test::MyClassPrx, p->ice_batchOneway());
#ifdef ICE_CPP11_MAPPING

    promise<void> prom;
    batch->ice_flushBatchRequestsAsync(nullptr,
        [&](bool sentSynchronously)
        {
            test(sentSynchronously);
            prom.set_value();
        }); // Empty flush
    prom.get_future().get();

    for(int i = 0; i < 30; ++i)
    {
        batch->opByteSOnewayAsync(bs1, nullptr, [](exception_ptr){ test(false); });
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
        shared_ptr<Test::MyClassPrx> batch1 = Ice::uncheckedCast<Test::MyClassPrx>(p->ice_batchOneway());
        shared_ptr<Test::MyClassPrx> batch2 = Ice::uncheckedCast<Test::MyClassPrx>(p->ice_batchOneway());

        batch1->ice_pingAsync().get();
        batch2->ice_pingAsync().get();
        batch1->ice_flushBatchRequestsAsync().get();
        batch1->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
        batch1->ice_pingAsync().get();
        batch2->ice_pingAsync().get();

        batch1->ice_getConnection();
        batch2->ice_getConnection();

        batch1->ice_pingAsync().get();
        batch1->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));

        batch1->ice_pingAsync().get();
        batch2->ice_pingAsync().get();
    }

    Ice::Identity identity;
    identity.name = "invalid";
    auto batch3 = batch->ice_identity(identity);
    batch3->ice_pingAsync();
    batch3->ice_flushBatchRequestsAsync().get();

    // Make sure that a bogus batch request doesn't cause troubles to other ones.
    batch3->ice_pingAsync();
    batch->ice_pingAsync();
    batch->ice_flushBatchRequestsAsync().get();
    batch->ice_pingAsync();
#else
    batch->end_ice_flushBatchRequests(batch->begin_ice_flushBatchRequests()); // Empty flush

    test(batch->begin_ice_flushBatchRequests()->isSent()); // Empty flush
    test(batch->begin_ice_flushBatchRequests()->isCompleted()); // Empty flush
    test(batch->begin_ice_flushBatchRequests()->sentSynchronously()); // Empty flush

    for(int i = 0; i < 30; ++i)
    {
        batch->begin_opByteSOneway(bs1, Test::newCallback_MyClass_opByteSOneway(new Callback_ByteSOneway(),
                                                                                &Callback_ByteSOneway::response,
                                                                                &Callback_ByteSOneway::exception));
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
        Test::MyClassPrx batch1 = Test::MyClassPrx::uncheckedCast(p->ice_batchOneway());
        Test::MyClassPrx batch2 = Test::MyClassPrx::uncheckedCast(p->ice_batchOneway());

        batch1->end_ice_ping(batch1->begin_ice_ping());
        batch2->end_ice_ping(batch2->begin_ice_ping());
        batch1->end_ice_flushBatchRequests(batch1->begin_ice_flushBatchRequests());
        batch1->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
        batch1->end_ice_ping(batch1->begin_ice_ping());
        batch2->end_ice_ping(batch2->begin_ice_ping());

        batch1->ice_getConnection();
        batch2->ice_getConnection();

        batch1->end_ice_ping(batch1->begin_ice_ping());
        batch1->ice_getConnection()->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));

        batch1->end_ice_ping(batch1->begin_ice_ping());
        batch2->end_ice_ping(batch2->begin_ice_ping());
    }

    Ice::Identity identity;
    identity.name = "invalid";
    Ice::ObjectPrx batch3 = batch->ice_identity(identity);
    batch3->begin_ice_ping();
    batch3->end_ice_flushBatchRequests(batch3->begin_ice_flushBatchRequests());

    // Make sure that a bogus batch request doesn't cause troubles to other ones.
    batch3->begin_ice_ping();
    batch->begin_ice_ping();
    batch->end_ice_flushBatchRequests(batch->begin_ice_flushBatchRequests());
    batch->begin_ice_ping();
#endif
}
