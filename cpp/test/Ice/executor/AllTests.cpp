// Copyright (c) ZeroC, Inc.

#include "../../src/Ice/Random.h"
#include "Executor.h"
#include "Ice/Ice.h"
#include "Test.h"
#include "TestHelper.h"

#include <chrono>

using namespace std;
using namespace Ice;
using namespace Test;
namespace
{
    class Callback
    {
    public:
        Callback() = default;

        void check()
        {
            unique_lock lock(_mutex);
            _condition.wait(lock, [this] { return _called; });
            _called = false;
        }

        void response()
        {
            test(Executor::isExecutorThread());
            called();
        }

        void exception(const Exception& ex)
        {
            test(dynamic_cast<const NoEndpointException*>(&ex));
            test(Executor::isExecutorThread());
            called();
        }

        void responseEx() { test(false); }

        void exceptionEx(const ::Exception& ex)
        {
            test(dynamic_cast<const InvocationTimeoutException*>(&ex));
            test(Executor::isExecutorThread());
            called();
        }

        void payload() { test(Executor::isExecutorThread()); }

        void ignoreEx(const Exception& ex) { test(dynamic_cast<const CommunicatorDestroyedException*>(&ex)); }

        void sent(bool sentSynchronously)
        {
            test(sentSynchronously || Executor::isExecutorThread());
            _sentSynchronously = sentSynchronously;
        }

        bool sentSynchronously() { return _sentSynchronously; }

    protected:
        void called()
        {
            lock_guard lock(_mutex);
            assert(!_called);
            _called = true;
            _condition.notify_one();
        }

    private:
        mutex _mutex;
        condition_variable _condition;
        bool _called{false};
        bool _sentSynchronously;
    };
    using CallbackPtr = shared_ptr<Callback>;
}

void
allTests(TestHelper* helper)
{
    CommunicatorPtr communicator = helper->communicator();

    TestIntfPrx p(communicator, "test:" + helper->getTestEndpoint());
    TestIntfControllerPrx testController(communicator, "testController:" + helper->getTestEndpoint(1, "tcp"));

    cout << "testing executor... " << flush;
    {
        p->op();

        CallbackPtr cb = make_shared<Callback>();
        p->opAsync(
            [cb]() { cb->response(); },
            [cb](exception_ptr err)
            {
                try
                {
                    rethrow_exception(err);
                }
                catch (const Exception& ex)
                {
                    cb->exception(ex);
                }
            });
        cb->check();

        auto i = p->ice_adapterId("dummy");
        i->opAsync(
            [cb]() { cb->response(); },
            [cb](exception_ptr err)
            {
                try
                {
                    rethrow_exception(err);
                }
                catch (const Exception& ex)
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
            to->sleepAsync(
                500,
                [cb]() { cb->responseEx(); },
                [cb](exception_ptr err)
                {
                    try
                    {
                        rethrow_exception(err);
                    }
                    catch (const Exception& ex)
                    {
                        cb->exceptionEx(ex);
                    }
                });
            cb->check();
        }
        testController->holdAdapter();

        ByteSeq seq;
        seq.resize(1024); // Make sure the request doesn't compress too well.
        for (auto& q : seq)
        {
            q = static_cast<byte>(IceInternal::random(255));
        }

        vector<shared_ptr<promise<void>>> completed;
        while (true)
        {
            auto s = make_shared<promise<bool>>();
            auto fs = s->get_future();
            auto c = make_shared<promise<void>>();

            p->opWithPayloadAsync(
                seq,
                [=]() { c->set_value(); },
                [=](exception_ptr) { c->set_value(); },
                [=](bool sent) { s->set_value(sent); });
            completed.push_back(c);

            if (fs.wait_for(chrono::milliseconds(0)) != future_status::ready || !fs.get())
            {
                break;
            }
        }
        testController->resumeAdapter();
        for (const auto& c : completed)
        {
            c->get_future().get();
        }
    }
    cout << "ok" << endl;

    p->shutdown();
}
