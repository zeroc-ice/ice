// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>

#include <CounterObserverI.h>

using namespace std;

CounterObserverI::CounterObserverI() :
    _value(0)
{
}

void
CounterObserverI::init(int value, const Ice::Current&)
{
    Lock sync(*this);
    _value = value;
    cout << "init: " << value << endl;
}

void
CounterObserverI::inc(int value, const Ice::Current&)
{
    Lock sync(*this);
    _value += value;
    cout << "int: " << value << " total: " << _value << endl;
}

