// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
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

class AMIWaitForSent : public Ice::AMISentCallback, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    AMIWaitForSent() : _sent(false)
    {
    }

    virtual void
    ice_sent()
    {
        Lock sync(*this);
        _sent = true;
        notify();
    }
    
    void
    waitForSent()
    {
        Lock sync(*this);
        while(!_sent)
        {
            wait();
        }
    }
    
private:

    bool _sent;
};

class AMICheckSetValue : public AMI_Hold_set, public AMIWaitForSent
{
public:
    
    AMICheckSetValue(const ConditionPtr& condition, Ice::Int expected) : _condition(condition), _expected(expected)
    {
    }

    virtual void
    ice_response(Ice::Int value)
    {
        if(value != _expected)
        {
            _condition->set(false);
        }
    }

    virtual void
    ice_exception(const Ice::Exception& ex)
    {
    }

private:

    const ConditionPtr _condition;
    Ice::Int _expected;
};
typedef IceUtil::Handle<AMICheckSetValue> AMICheckSetValuePtr;

}

void
allTests(const Ice::CommunicatorPtr& communicator)
{
    cout << "testing stringToProxy... " << flush;
    string ref = "hold:default -p 12010";
    Ice::ObjectPrx base = communicator->stringToProxy(ref);
    test(base);
    string refSerialized = "hold:default -p 12011";
    Ice::ObjectPrx baseSerialized = communicator->stringToProxy(refSerialized);
    test(base);
    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;
    HoldPrx hold = HoldPrx::checkedCast(base);
    test(hold);
    test(hold == base);
    HoldPrx holdSerialized = HoldPrx::checkedCast(baseSerialized);
    test(holdSerialized);
    test(holdSerialized == baseSerialized);
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
        AMICheckSetValuePtr cb;
        while(cond->value())
        {
            cb = new AMICheckSetValue(cond, value);
            if(hold->set_async(cb, ++value, IceUtilInternal::random(5)))
            {
                cb = 0;
            }
            if(value % 100 == 0)
            {
                if(cb)
                {
                    cb->waitForSent();
                    cb = 0;
                }
            }
        }
        if(cb)
        {
            cb->waitForSent();
            cb = 0;
        }
    }
    cout << "ok" << endl;

    cout << "testing with serialize mode... " << flush;
    {
        ConditionPtr cond = new Condition(true);
        int value = 0;
        AMICheckSetValuePtr cb;
        while(value < 3000 && cond->value())
        {
            cb = new AMICheckSetValue(cond, value);
            if(holdSerialized->set_async(cb, ++value, 0))
            {
                cb = 0;
            }
            if(value % 100 == 0)
            {
                if(cb)
                {
                    cb->waitForSent();
                    cb = 0;
                }
            }
        }
        if(cb)
        {
            cb->waitForSent();
            cb = 0;
        }
        test(cond->value());

        for(int i = 0; i < 20000; ++i)
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
