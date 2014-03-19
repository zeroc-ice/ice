// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>

#include <CounterObserverI.h>

using namespace std;

CounterObserverI::CounterObserverI(const MTPrinterPtr& printer) :
    _value(0),
    _printer(printer)
{
}

void
CounterObserverI::init(int value, const Ice::Current&)
{
    IceUtil::Mutex::Lock sync(_mutex);
    _value = value;
    ostringstream ostr;
    ostr << "init: " << value << endl;
    _printer->print(ostr.str());
}

void
CounterObserverI::inc(int value, const Ice::Current&)
{
    IceUtil::Mutex::Lock sync(_mutex);
    _value += value;
    ostringstream ostr;
    ostr << "int: " << value << " total: " << _value << endl;
    _printer->print(ostr.str());
}

