// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef HELLO_I_H
#define HELLO_I_H

#include <Hello.h>
#include <WorkQueue.h>

class HelloI : virtual public Demo::Hello
{
public:

    HelloI(const WorkQueuePtr&);

    virtual void sayHello_async(const Demo::AMD_Hello_sayHelloPtr&, int, const Ice::Current&);
    virtual void shutdown(const Ice::Current&);

private:

    WorkQueuePtr _workQueue;
};


#endif
