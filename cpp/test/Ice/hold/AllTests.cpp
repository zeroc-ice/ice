// Copyright (c) ZeroC, Inc.

#include "../../src/Ice/Random.h"
#include "Ice/Ice.h"
#include "Test.h"
#include "TestHelper.h"

#include <chrono>
#include <thread>

using namespace std;
using namespace Ice;
using namespace Test;

namespace
{
    class Condition
    {
    public:
        Condition(bool value) : _value(value) {}

        void set(bool value)
        {
            lock_guard lock(_mutex);
            _value = value;
        }

        [[nodiscard]] bool value() const
        {
            lock_guard lock(_mutex);
            return _value;
        }

    private:
        bool _value;
        mutable mutex _mutex;
    };
    using ConditionPtr = shared_ptr<Condition>;
}

void
allTests(Test::TestHelper* helper)
{
    CommunicatorPtr communicator = helper->communicator();
    HoldPrx hold(communicator, "hold:" + helper->getTestEndpoint());
    HoldPrx holdSerialized(communicator, "hold:" + helper->getTestEndpoint(1));

    cout << "changing state between active and hold rapidly... " << flush;
    for (int i = 0; i < 100; ++i)
    {
        hold->putOnHold(0);
    }
    for (int i = 0; i < 100; ++i)
    {
        hold->ice_oneway()->putOnHold(0);
    }
    for (int i = 0; i < 100; ++i)
    {
        holdSerialized->putOnHold(0);
    }
    for (int i = 0; i < 100; ++i)
    {
        holdSerialized->ice_oneway()->putOnHold(0);
    }
    cout << "ok" << endl;

    cout << "testing without serialize mode... " << flush;
    {
        ConditionPtr cond = make_shared<Condition>(true);
        int value = 0;
        shared_ptr<promise<void>> completed;
        while (cond->value())
        {
            completed = make_shared<promise<void>>();
            auto sent = make_shared<promise<bool>>();
            auto expected = value;
            hold->setAsync(
                value + 1,
                static_cast<int32_t>(IceInternal::random(5)),
                [cond, expected, completed](int val)
                {
                    if (val != expected)
                    {
                        cond->set(false);
                    }
                    completed->set_value();
                },
                [completed](exception_ptr) { completed->set_value(); },
                [sent](bool sentSynchronously) { sent->set_value(sentSynchronously); });

            ++value;
            if (value % 100 == 0)
            {
                sent->get_future().get();
            }

            if (value > 1000000)
            {
                // Don't continue, it's possible that out-of-order dispatch doesn't occur
                // after 100000 iterations and we don't want the test to last for too long
                // when this occurs.
                break;
            }
        }
        test(value > 100000 || !cond->value());
        completed->get_future().get();
    }
    cout << "ok" << endl;

    cout << "testing with serialize mode... " << flush;
    {
        ConditionPtr cond = make_shared<Condition>(true);
        int value = 0;
        shared_ptr<promise<void>> completed;
        while (value < 3000 && cond->value())
        {
            completed = make_shared<promise<void>>();
            auto sent = make_shared<promise<bool>>();
            auto expected = value;
            holdSerialized->setAsync(
                value + 1,
                static_cast<int32_t>(IceInternal::random(1)),
                [cond, expected, completed](int val)
                {
                    if (val != expected)
                    {
                        cond->set(false);
                    }
                    completed->set_value();
                },
                [completed](exception_ptr) { completed->set_value(); },
                [sent](bool sentSynchronously) { sent->set_value(sentSynchronously); });
            ++value;
            if (value % 100 == 0)
            {
                sent->get_future().get();
            }
        }
        test(cond->value());
        for (int i = 0; i < 1000; ++i)
        {
            holdSerialized->ice_oneway()->setOneway(value + 1, value);
            ++value;
            if ((i % 10) == 0)
            {
                holdSerialized->ice_oneway()->putOnHold(1);
            }
        }
    }
    cout << "ok" << endl;

    cout << "testing serialization... " << flush;
    {
        int value = 0;
        holdSerialized->set(value, 0);
        // We use the same proxy for all oneway calls.
        auto holdSerializedOneway = holdSerialized->ice_oneway();

        shared_ptr<promise<void>> completed;
        for (int i = 0; i < 10000; ++i)
        {
            completed = make_shared<promise<void>>();
            holdSerializedOneway->setOnewayAsync(
                value + 1,
                value,
                nullptr,
                [](exception_ptr) {},
                [completed](bool /*sentSynchronously*/) { completed->set_value(); });
            ++value;
            if ((i % 100) == 0)
            {
                completed->get_future().get();
                holdSerialized->ice_ping(); // Ensure everything's dispatched
                holdSerialized->ice_getConnection()->close().get();
            }
        }
        completed->get_future().get();
    }
    cout << "ok" << endl;

    cout << "testing waitForHold... " << flush;
    {
        hold->waitForHold();
        hold->waitForHold();
        for (int i = 0; i < 1000; ++i)
        {
            hold->ice_oneway()->ice_ping();
            if ((i % 20) == 0)
            {
                hold->putOnHold(0);
            }
        }
        hold->putOnHold(-1);
        hold->ice_ping();
        hold->putOnHold(-1);
        hold->ice_ping();
    }
    cout << "ok" << endl;

    cout << "testing hold with idle timeout... " << flush;
    ConnectionPtr connection = hold->ice_getCachedConnection();
    hold->putOnHold(2000); // despite the name, this reactivates the adapter in about 2s
    hold->putOnHold(-1);   // immediate hold
    this_thread::sleep_for(chrono::milliseconds(2000));
    hold->ice_ping(); // goes through when the first putOnHold reactivates the adapter
    test(connection == hold->ice_getCachedConnection());
    cout << "ok" << endl;

    cout << "changing state to hold and shutting down server... " << flush;
    hold->shutdown();
    cout << "ok" << endl;
}
