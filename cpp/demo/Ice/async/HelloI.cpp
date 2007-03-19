// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <HelloI.h>

using namespace std;

HelloI::HelloI(const WorkQueuePtr& workQueue) :
    _workQueue(workQueue)
{
}

void
HelloI::sayHello_async(const Demo::AMD_Hello_sayHelloPtr& cb, int delay, const Ice::Current&)
{
    if(delay == 0)
    {
        cout << "Hello World!" << endl;
        cb->ice_response();
    }
    else
    {
        _workQueue->add(cb, delay);
    }
}

void
HelloI::shutdown(const Ice::Current& curr)
{
    cout << "Shutting down..." << endl;

    _workQueue->destroy();
    curr.adapter->getCommunicator()->shutdown();
}
