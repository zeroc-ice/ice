// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceUtil/Random.h>
#include <TestCommon.h>
#include <Test.h>
#include <Dispatcher.h>

using namespace std;

namespace
{

class Callback : public ICE_ENABLE_SHARED_FROM_THIS(Callback)
{
public:

    Callback() :
        _called(false)
    {
    }

    void check()
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_m);
        while(!_called)
        {
            _m.wait();
        }
        _called = false;
    }

    void
    response()
    {
        test(Dispatcher::isDispatcherThread());
        called();
    }

    void
    exception(const Ice::Exception& ex)
    {
        test(dynamic_cast<const Ice::NoEndpointException*>(&ex));
        test(Dispatcher::isDispatcherThread());
        called();
    }

    void responseEx()
    {
        test(false);
    }

    void exceptionEx(const ::Ice::Exception& ex)
    {
        test(dynamic_cast<const Ice::InvocationTimeoutException*>(&ex));
        test(Dispatcher::isDispatcherThread());
        called();
    }

    void
    payload()
    {
        test(Dispatcher::isDispatcherThread());
    }

    void
    ignoreEx(const Ice::Exception& ex)
    {
        test(dynamic_cast<const Ice::CommunicatorDestroyedException*>(&ex));
    }

    void
    sent(bool sentSynchronously)
    {
        test(sentSynchronously || Dispatcher::isDispatcherThread());
        _sentSynchronously = sentSynchronously;
    }
    
    bool
    sentSynchronously()
    {
        return _sentSynchronously;
    }

protected:

    void called()
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_m);
        assert(!_called);
        _called = true;
        _m.notify();
    }

private:

    IceUtil::Monitor<IceUtil::Mutex> _m;
    bool _called;
    bool _sentSynchronously;
};
ICE_DEFINE_PTR(CallbackPtr, Callback);

}

void
allTests(const Ice::CommunicatorPtr& communicator)
{
    string sref = "test:" + getTestEndpoint(communicator, 0);
    Ice::ObjectPrxPtr obj = communicator->stringToProxy(sref);
    test(obj);

    Test::TestIntfPrxPtr p = ICE_UNCHECKED_CAST(Test::TestIntfPrx, obj);

    sref = "testController:" + getTestEndpoint(communicator, 1, "tcp");
    obj = communicator->stringToProxy(sref);
    test(obj);

    Test::TestIntfControllerPrxPtr testController = ICE_UNCHECKED_CAST(Test::TestIntfControllerPrx, obj);

#ifdef ICE_CPP11_COMPILER
    cout << "testing C++11 dispatcher... " << flush;
#else
    cout << "testing dispatcher... " << flush;
#endif
    {
        p->op();

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
#ifdef ICE_CPP11_MAPPING
        p->op_async(
            [cb]()
            {
                cb->response();
            },
            [cb](exception_ptr err)
            {
                try
                {
                    rethrow_exception(err);
                }
                catch(const Ice::Exception& ex)
                {
                    cb->exception(ex);
                }
            });
        cb->check();
        
        auto i = p->ice_adapterId("dummy");
        i->op_async(
            [cb]()
            {
                cb->response();
            },
            [cb](exception_ptr err)
            {
                try
                {
                    rethrow_exception(err);
                }
                catch(const Ice::Exception& ex)
                {
                    cb->exception(ex);
                }
            });
        cb->check();
        
        {
            //
            // Expect InvocationTimeoutException.
            //
            auto to = p->ice_invocationTimeout(250);
            to->sleep_async(500,
                [cb]()
                {
                    cb->responseEx();
                },
                [cb](exception_ptr err)
                {
                    try
                    {
                        rethrow_exception(err);
                    }
                    catch(const Ice::Exception& ex)
                    {
                        cb->exceptionEx(ex);
                    }
                });
            cb->check();
        }

        testController->holdAdapter();

        Ice::ByteSeq seq;
        seq.resize(1024); // Make sure the request doesn't compress too well.
        for(Ice::ByteSeq::iterator q = seq.begin(); q != seq.end(); ++q)
        {
            *q = static_cast<Ice::Byte>(IceUtilInternal::random(255));
        }

        promise<void> completed;
        while(true)
        {
            CallbackPtr cb = ICE_MAKE_SHARED(Callback);
            promise<bool> call;
            future<bool> sent = call.get_future();
            p->opWithPayload_async(seq, 
                [cb, &sent, &completed]()
                {
                    cb->payload();
                    if(!cb->sentSynchronously())
                    {
                        completed.set_value();
                    }
                },
                [cb](exception_ptr err)
                {
                    try
                    {
                        rethrow_exception(err);
                    }
                    catch(const Ice::Exception& ex)
                    {
                        cb->ignoreEx(ex);
                    }
                },
                [cb, &call](bool sentSynchronously)
                {
                    cb->sent(sentSynchronously);
                    call.set_value(sentSynchronously);
                });
            if(!sent.get())
            {
                break;
            }
        }
        testController->resumeAdapter();
        completed.get_future().get();
#else
        Test::Callback_TestIntf_opPtr callback = Test::newCallback_TestIntf_op(cb,
                                                                               &Callback::response,
                                                                               &Callback::exception);
        p->begin_op(callback);
        cb->check();

        Test::TestIntfPrx i = p->ice_adapterId("dummy");
        i->begin_op(callback);
        cb->check();

        {
            //
            // Expect InvocationTimeoutException.
            //
            Test::TestIntfPrx to = p->ice_invocationTimeout(250);
            to->begin_sleep(500, Test::newCallback_TestIntf_sleep(cb, &Callback::responseEx, &Callback::exceptionEx));
            cb->check();
        }

        testController->holdAdapter();

        Test::Callback_TestIntf_opWithPayloadPtr callback2 =
            Test::newCallback_TestIntf_opWithPayload(cb, &Callback::payload, &Callback::ignoreEx, &Callback::sent);

        Ice::ByteSeq seq;
        seq.resize(1024); // Make sure the request doesn't compress too well.
        for(Ice::ByteSeq::iterator q = seq.begin(); q != seq.end(); ++q)
        {
            *q = static_cast<Ice::Byte>(IceUtilInternal::random(255));
        }
        Ice::AsyncResultPtr result;
        while((result = p->begin_opWithPayload(seq, callback2))->sentSynchronously());
        testController->resumeAdapter();
        result->waitForCompleted();
#endif
    }
    cout << "ok" << endl;

    p->shutdown();
}
