// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceUtil/Random.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;
using namespace Test;

namespace
{

class Condition : public IceUtil::Mutex, public IceUtil::Shared
{
public:

    Condition(bool value) : _value(value)
    {
    }

    void
    set(bool value)
    {
        Lock sync(*this);
        _value = value;
    }

    bool
    value() const
    {
        Lock sync(*this);
        return _value;
    }

private:

    bool _value;
};
typedef IceUtil::Handle<Condition> ConditionPtr;

class SetCB : public IceUtil::Shared, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    SetCB(const ConditionPtr& condition, Ice::Int expected) : _condition(condition), _expected(expected)
    {
    }

    void
    response(Ice::Int value)
    {
        if(value != _expected)
        {
            _condition->set(false);
        }
    }

    void
    exception(const Ice::Exception&)
    {
    }

private:

    const ConditionPtr _condition;
    Ice::Int _expected;
};
typedef IceUtil::Handle<SetCB> SetCBPtr;

}

void
allTests(const Ice::CommunicatorPtr& communicator)
{
    cout << "testing stringToProxy... " << flush;
    string ref = "hold:" + getTestEndpoint(communicator, 0);
    Ice::ObjectPrxPtr base = communicator->stringToProxy(ref);
    test(base);
    string refSerialized = "hold:" + getTestEndpoint(communicator, 1);
    Ice::ObjectPrxPtr baseSerialized = communicator->stringToProxy(refSerialized);
    test(base);
    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;
    HoldPrxPtr hold = ICE_CHECKED_CAST(HoldPrx, base);
    test(hold);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEquals(hold, base));
#else
    test(hold == base);
#endif
    HoldPrxPtr holdSerialized = ICE_CHECKED_CAST(HoldPrx, baseSerialized);
    test(holdSerialized);

#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEquals(holdSerialized, baseSerialized));
#else
    test(holdSerialized == baseSerialized);
#endif
    cout << "ok" << endl;

    cout << "changing state between active and hold rapidly... " << flush;
    int i;
    for(i = 0; i < 100; ++i)
    {
        hold->putOnHold(0);
    }
    for(i = 0; i < 100; ++i)
    {
        hold->ice_oneway()->putOnHold(0);
    }
    for(i = 0; i < 100; ++i)
    {
        holdSerialized->putOnHold(0);
    }
    for(i = 0; i < 100; ++i)
    {
        holdSerialized->ice_oneway()->putOnHold(0);
    }
    cout << "ok" << endl;

    cout << "testing without serialize mode... " << flush;
    {
        ConditionPtr cond = new Condition(true);
        int value = 0;
#ifdef ICE_CPP11_MAPPING
        shared_ptr<promise<void>> completed;
        while(cond->value())
        {
            completed = make_shared<promise<void>>();
            auto sent = make_shared<promise<bool>>();
            auto expected = value;
            hold->setAsync(value + 1, IceUtilInternal::random(5),
                [cond, expected, completed](int value)
                {
                    if(value != expected)
                    {
                        cond->set(false);
                    }
                    completed->set_value();
                },
                [completed](exception_ptr)
                {
                    completed->set_value();
                },
                [sent](bool sentSynchronously)
                {
                    sent->set_value(sentSynchronously);
                });

            ++value;
            if(value % 100 == 0)
            {
                sent->get_future().get();
            }

            if(value > 1000000)
            {
                // Don't continue, it's possible that out-of-order dispatch doesn't occur
                // after 100000 iterations and we don't want the test to last for too long
                // when this occurs.
                break;
            }
        }
        test(value > 100000 || !cond->value());
        completed->get_future().get();
#else

        Ice::AsyncResultPtr result;
        while(cond->value())
        {
            result = hold->begin_set(value + 1,
                                     IceUtilInternal::random(5),
                                     newCallback_Hold_set(new SetCB(cond, value), &SetCB::response, &SetCB::exception));
            ++value;
            if(value % 100 == 0)
            {
                result->waitForSent();
            }

            if(value > 1000000)
            {
                // Don't continue, it's possible that out-of-order dispatch doesn't occur
                // after 100000 iterations and we don't want the test to last for too long
                // when this occurs.
                break;
            }
        }
        test(value > 100000 || !cond->value());
        result->waitForCompleted();
#endif
    }
    cout << "ok" << endl;

    cout << "testing with serialize mode... " << flush;
    {
        ConditionPtr cond = new Condition(true);
        int value = 0;
#ifdef ICE_CPP11_MAPPING
        shared_ptr<promise<void>> completed;
        while(value < 3000 && cond->value())
        {
            completed = make_shared<promise<void>>();
            auto sent = make_shared<promise<bool>>();
            auto expected = value;
            holdSerialized->setAsync(
                value + 1,
                IceUtilInternal::random(1),
                [cond, expected, completed](int value)
                {
                    if(value != expected)
                    {
                        cond->set(false);
                    }
                    completed->set_value();
                },
                [completed](exception_ptr)
                {
                    completed->set_value();
                },
                [sent](bool sentSynchronously)
                {
                    sent->set_value(sentSynchronously);
                });
            ++value;
            if(value % 100 == 0)
            {
                sent->get_future().get();
            }
        }
#else
        Ice::AsyncResultPtr result;
        while(value < 3000 && cond->value())
        {
            result = holdSerialized->begin_set(value + 1,
                                               IceUtilInternal::random(1),
                                               newCallback_Hold_set(new SetCB(cond, value),
                                                                    &SetCB::response,
                                                                    &SetCB::exception));
            ++value;
            if(value % 100 == 0)
            {
                result->waitForSent();
            }
        }
        result->waitForCompleted();
#endif
        test(cond->value());
        for(int i = 0; i < 10000; ++i)
        {
            holdSerialized->ice_oneway()->setOneway(value + 1, value);
            ++value;
            if((i % 100) == 0)
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
#ifdef ICE_CPP11_MAPPING
        shared_ptr<promise<void>> completed;
        for(int i = 0; i < 10000; ++i)
        {
            completed = make_shared<promise<void>>();
            // Create a new proxy for each request
            holdSerialized->ice_oneway()->setOnewayAsync(value + 1, value,
                nullptr,
                [](exception_ptr)
                {
                },
                [completed](bool sentSynchronously)
                {
                    completed->set_value();
                });
            ++value;
            if((i % 100) == 0)
            {
                completed->get_future().get();
                holdSerialized->ice_ping(); // Ensure everything's dispatched
                holdSerialized->ice_getConnection()->close(false);
            }
        }
        completed->get_future().get();
#else
        Ice::AsyncResultPtr result;
        for(int i = 0; i < 10000; ++i)
        {
            // Create a new proxy for each request
            result = holdSerialized->ice_oneway()->begin_setOneway(value + 1, value);
            ++value;
            if((i % 100) == 0)
            {
                result->waitForSent();
                holdSerialized->ice_ping(); // Ensure everything's dispatched
                holdSerialized->ice_getConnection()->close(false);
            }
        }
        result->waitForCompleted();
#endif
    }
    cout << "ok" << endl;

    cout << "testing waitForHold... " << flush;
    {
        hold->waitForHold();
        hold->waitForHold();
        for(i = 0; i < 1000; ++i)
        {
            hold->ice_oneway()->ice_ping();
            if((i % 20) == 0)
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

    cout << "changing state to hold and shutting down server... " << flush;
    hold->shutdown();
    cout << "ok" << endl;
}
