// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef COUNTER_I_H
#define COUNTER_I_H

#include <IceStorm/IceStorm.h>
#include <Counter.h>

class CounterI : public Demo::Counter, public IceUtil::Mutex
{
public:

    CounterI(const IceStorm::TopicPrx&);

    virtual void subscribe(const Demo::CounterObserverPrx& observer, const Ice::Current&);
    virtual void unsubscribe(const Demo::CounterObserverPrx& observer, const Ice::Current&);
    virtual void inc(int value, const Ice::Current&);

private:

    int _value;
    IceStorm::TopicPrx _topic;
    Demo::CounterObserverPrx _publisher;
};

#endif
