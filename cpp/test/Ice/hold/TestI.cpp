// Copyright (c) ZeroC, Inc.

#include "TestI.h"
#include "Ice/Ice.h"
#include "TestHelper.h"

#include <chrono>
#include <exception>
#include <iostream>
#include <thread>

using namespace std;

HoldI::HoldI(Ice::ObjectAdapterPtr adapter) : _adapter(std::move(adapter)) {}

void
HoldI::putOnHoldAsync(int32_t delay, function<void()> response, function<void(std::exception_ptr)>, const Ice::Current&)
{
    if (delay < 0)
    {
        _adapter->hold();
        response();
    }
    else if (delay == 0)
    {
        _adapter->hold();
        _adapter->activate();
        response();
    }
    else
    {
        response();
        this_thread::sleep_for(chrono::milliseconds(delay));
        try
        {
            lock_guard lock(_taskMutex); // serialize background tasks
            _adapter->hold();
            _adapter->activate();
        }
        catch (const std::exception& ex)
        {
            // unexpected
            cerr << "error: " << ex.what() << endl;
            test(false);
        }
    }
}

void
HoldI::waitForHoldAsync(function<void()> response, function<void(std::exception_ptr)>, const Ice::Current& current)
{
    response();

    try
    {
        lock_guard lock(_taskMutex); // serialize background tasks
        current.adapter->waitForHold();
        current.adapter->activate();
    }
    catch (const std::exception& ex)
    {
        // unexpected
        cerr << "error: " << ex.what() << endl;
        test(false);
    }
}

int32_t
HoldI::set(int32_t value, int32_t delay, const Ice::Current&)
{
    this_thread::sleep_for(chrono::milliseconds(delay));

    lock_guard lock(_mutex);
    int32_t tmp = _last;
    _last = value;
    return tmp;
}

void
HoldI::setOneway(int32_t value, int32_t expected, const Ice::Current&)
{
    lock_guard lock(_mutex);
    test(_last == expected);
    _last = value;
}

void
HoldI::shutdown(const Ice::Current&)
{
    _adapter->hold();
    _adapter->getCommunicator()->shutdown();
}
