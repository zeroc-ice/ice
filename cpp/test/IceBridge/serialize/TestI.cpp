//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "TestI.h"
#include "Ice/Ice.h"
#include "TestHelper.h"

using namespace std;
using namespace Test;

void
MyClassI::incCounter(int expected, const Ice::Current&)
{
    {
        lock_guard<mutex> lg(_mutex);
        if (_counter + 1 != expected)
        {
            cout << _counter << " " << expected << endl;
        }
        test(++_counter == expected);
    }
    _condVar.notify_all();
}

void
MyClassI::waitCounter(int value, const Ice::Current&)
{
    unique_lock<mutex> lock(_mutex);
    while (_counter != value)
    {
        _condVar.wait(lock);
    }
}

void
MyClassI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}
