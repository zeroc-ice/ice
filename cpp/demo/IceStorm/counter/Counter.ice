// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

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

