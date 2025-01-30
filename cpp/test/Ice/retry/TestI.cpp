// Copyright (c) ZeroC, Inc.

#include "TestI.h"
#include "Ice/Ice.h"

#include <thread>
using namespace std;

RetryI::RetryI() = default;

void
RetryI::op(bool kill, const Ice::Current& current)
{
    if (kill)
    {
        if (current.con)
        {
            current.con->abort();
        }
        else
        {
            throw Ice::ConnectionLostException{__FILE__, __LINE__, 0};
        }
    }
}

int
RetryI::opIdempotent(int nRetry, const Ice::Current&)
{
    if (nRetry < 0)
    {
        _counter = 0;
        return 0;
    }

    if (nRetry > _counter)
    {
        ++_counter;
        throw Ice::ConnectionLostException{__FILE__, __LINE__, 0};
    }
    int counter = _counter;
    _counter = 0;
    return counter;
}

void
RetryI::opNotIdempotent(const Ice::Current&)
{
    throw Ice::ConnectionLostException{__FILE__, __LINE__, 0};
}

void
RetryI::sleep(int delay, const Ice::Current&)
{
    this_thread::sleep_for(chrono::milliseconds(delay));
}

void
RetryI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}
