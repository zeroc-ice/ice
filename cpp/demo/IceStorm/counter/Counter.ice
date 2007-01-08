// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef COUNTER_ICE
#define COUNTER_ICE

module Demo
{

interface CounterObserver
{
    void init(int value);
    void inc(int value);
};

interface Counter
{
    void subscribe(CounterObserver* observer);
    void unsubscribe(CounterObserver* observer);
    void inc(int value);
};

};

#endif
