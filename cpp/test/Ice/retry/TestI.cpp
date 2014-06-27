// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestI.h>
#include <SystemFailure.h>

namespace
{

const int nRetry = 4; // See Ice.RetryIntervals configuration in Client.cpp/Collocated.cpp

}

RetryI::RetryI() : _counter(0)
{
}

void
RetryI::op(bool kill, const Ice::Current& current)
{
    if(kill)
    {
        if(current.con)
        {
            current.con->close(true);
        }
        else
        {
            throw Ice::ConnectionLostException(__FILE__, __LINE__);
        }
    }
}

int
RetryI::opIdempotent(int counter, const Ice::Current& current)
{
    if(counter + nRetry > _counter)
    {
        ++_counter;
        if(current.con)
        {
            current.con->close(true);
        }
        else
        {
            throw Ice::ConnectionLostException(__FILE__, __LINE__);
        }
    }
    return _counter;
}

void
RetryI::opNotIdempotent(int counter, const Ice::Current& current)
{
    if(_counter != counter)
    {
        return;
    }

    ++_counter;
    if(current.con)
    {
        current.con->close(true);
    }
    else
    {
        throw Ice::ConnectionLostException(__FILE__, __LINE__);
    }
}

void
RetryI::opSystemException(const Ice::Current&)
{
    throw SystemFailure(__FILE__, __LINE__);
}

void
RetryI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}
