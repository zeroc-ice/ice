// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>

#include <CounterI.h>

using namespace std;
using namespace Demo;

CounterI::CounterI(const IceStorm::TopicPrx& topic) :
    _value(0),
    _topic(topic),
    _publisher(CounterObserverPrx::uncheckedCast(topic->getPublisher()))
{
}

void
CounterI::subscribe(const CounterObserverPrx& observer, const Ice::Current&)
{
    Lock sync(*this);

    //
    // Subscribe to the IceStorm topic. This returns a per-subscriber
    // object which is then used to send the initialize event to just
    // the given subscriber.
    //
    CounterObserverPrx o = CounterObserverPrx::uncheckedCast(
        _topic->subscribeAndGetPublisher(IceStorm::QoS(), observer));
    o->init(_value);
}

void
CounterI::unsubscribe(const CounterObserverPrx& observer, const Ice::Current&)
{
    _topic->unsubscribe(observer);
}

void
CounterI::inc(int value, const Ice::Current&)
{
    Lock sync(*this);

    _value += value;
    _publisher->inc(value);
}
