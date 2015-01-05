// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
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

class Callback_ByteSOneway1 : public IceUtil::Shared
{
    CallbackPtr _cb;

public:

    Callback_ByteSOneway1(const CallbackPtr& cb) : _cb(cb)
    {
    }

    void response()
    {
        _cb->called();
    }

    void exception(const ::Ice::Exception&)
    {
        test(false);
    }
};

class Callback_ByteSOneway3 : public IceUtil::Shared
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
batchOnewaysAMI(const Test::MyClassPrx& p)
{
    const Test::ByteS bs1(10  * 1024);
    const Test::ByteS bs2(99  * 1024);
    const Test::ByteS bs3(100  * 1024);
    
    CallbackPtr cb = new Callback();
    p->begin_opByteSOneway(bs1, Test::newCallback_MyClass_opByteSOneway(new Callback_ByteSOneway1(cb),
        &Callback_ByteSOneway1::response, &Callback_ByteSOneway1::exception));
    cb->check();

    p->begin_opByteSOneway(bs2, Test::newCallback_MyClass_opByteSOneway(new Callback_ByteSOneway1(cb),
        &Callback_ByteSOneway1::response, &Callback_ByteSOneway1::exception));
    cb->check();

    Test::MyClassPrx batch = Test::MyClassPrx::uncheckedCast(p->ice_batchOneway());
    batch->end_ice_flushBatchRequests(batch->begin_ice_flushBatchRequests());

    int i;

    for(i = 0 ; i < 30 ; ++i)
    {
        p->begin_opByteSOneway(bs1, Test::newCallback_MyClass_opByteSOneway(new Callback_ByteSOneway3(), 
                                                                            &Callback_ByteSOneway3::response, 
                                                                            &Callback_ByteSOneway3::exception));
    }
    
    if(batch->ice_getConnection())
    {
        batch->ice_getConnection()->end_flushBatchRequests(batch->ice_getConnection()->begin_flushBatchRequests());

        Test::MyClassPrx batch2 = Test::MyClassPrx::uncheckedCast(p->ice_batchOneway());

        batch->begin_ice_ping();
        batch2->begin_ice_ping();
        batch->end_ice_flushBatchRequests(batch->begin_ice_flushBatchRequests());
        batch->ice_getConnection()->close(false);
        batch->begin_ice_ping();
        batch2->begin_ice_ping();

        batch->ice_getConnection();
        batch2->ice_getConnection();

        batch->begin_ice_ping();
        batch->ice_getConnection()->close(false);

        batch->begin_ice_ping(Ice::newCallback_Object_ice_ping(new Callback_ping(cb), &Callback_ping::response, &Callback_ping::exception));
        cb->check();

        batch2->begin_ice_ping(Ice::newCallback_Object_ice_ping(new Callback_ping(cb), &Callback_ping::response, &Callback_ping::exception));
        cb->check();

        batch->begin_ice_ping();
        batch2->begin_ice_ping();
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
}
