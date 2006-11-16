// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef QUEUE_I_H
#define QUEUE_I_H

#include <IceUtil/Mutex.h>
#include <Queue.h>
#include <list>

class QueueI : virtual public Demo::Queue, public IceUtil::Mutex
{
public:

    virtual void get_async(const Demo::AMD_Queue_getPtr&, const Ice::Current&);
    virtual void add(const std::string&, const Ice::Current&);


private:

    std::list<std::string> _messageQueue;
    std::list<Demo::AMD_Queue_getPtr> _requestQueue;
};


#endif
