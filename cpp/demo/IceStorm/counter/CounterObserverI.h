// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef COUNTER_OBSERVER_I
#define COUNTER_OBSERVER_I

#include <Counter.h>

class CounterObserverI : public Demo::CounterObserver, public IceUtil::Mutex
{
public:

    CounterObserverI();

    virtual void init(int, const Ice::Current&);
    virtual void inc(int, const Ice::Current&);

private:

    int _value;
};

#endif
