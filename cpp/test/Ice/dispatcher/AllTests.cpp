//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <IceUtil/Random.h>
#include <TestHelper.h>
#include <Test.h>
#include <Dispatcher.h>

using namespace std;

namespace
{

class Callback
#ifndef ICE_CPP11_MAPPING
    : public IceUtil::Shared
#endif
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
allTests(Test::TestHelper* helper)
{
    Ice::CommunicatorPtr communicator = helper->communicator();
    string sref = "test:" + helper->getTestEndpoint();
    Ice::ObjectPrxPtr obj = communicator->stringToProxy(sref);
    test(obj);

    Test::TestIntfPrxPtr p = ICE_UNCHECKED_CAST(Test::TestIntfPrx, obj);

    sref = "testController:" + helper->getTestEndpoint(1, "tcp");
    obj = communicator->stringToProxy(sref);
    test(obj);

    Test::TestIntfControllerPrxPtr testController = ICE_UNCHECKED_CAST(Test::TestIntfControllerPrx, obj);

    cout << "testing dispatcher... " << flush;
    {
        p->op();

        CallbackPtr cb = ICE_MAKE_SHARED(Callback);
#ifdef ICE_CPP11_MAPPING
        p->opAsync(
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
        i->opAsync(
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
            auto to = p->ice_invocationTimeout(10);
            to->sleepAsync(500,
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

        vector<shared_ptr<promise<void>>> completed;
        while(true)
        {
            auto s = make_shared<promise<bool>>();
            auto fs = s->get_future();
            auto c = make_shared<promise<void>>();

            p->opWithPayloadAsync(seq,
                [=]()
                {
                    c->set_value();
                },
                [=](exception_ptr)
                {
                    c->set_value();
                },
                [=](bool sent)
                {
                    s->set_value(sent);
                });
            completed.push_back(c);

            if(fs.wait_for(chrono::milliseconds(0)) != future_status::ready || !fs.get())
            {
                break;
            }
        }
        testController->resumeAdapter();
        for(auto& c : completed)
        {
            c->get_future().get();
        }
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
            Test::TestIntfPrx to = p->ice_invocationTimeout(10);
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
