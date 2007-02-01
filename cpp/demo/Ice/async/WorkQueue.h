// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef WORK_QUEUE_H
#define WORK_QUEUE_H

#include <Hello.h>
#include <IceUtil/Thread.h>
#include <IceUtil/Monitor.h>
#include <IceUtil/Mutex.h>

#include <list>

class WorkQueue : public IceUtil::Thread
{
public:

    WorkQueue();

    virtual void run();

    void add(const Demo::AMD_Hello_sayHelloPtr&, int);
    void destroy();

private:

    struct CallbackEntry
    {
        Demo::AMD_Hello_sayHelloPtr cb;
        int delay;
    };

    IceUtil::Monitor<IceUtil::Mutex> _monitor;
    std::list<CallbackEntry> _callbacks;
    bool _done;
};

typedef IceUtil::Handle<WorkQueue> WorkQueuePtr;

#endif
