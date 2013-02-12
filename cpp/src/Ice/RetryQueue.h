// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_RETRY_QUEUE_H
#define ICE_RETRY_QUEUE_H

#include <IceUtil/Shared.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/Timer.h>
#include <Ice/RetryQueueF.h>
#include <Ice/OutgoingAsyncF.h>
#include <Ice/InstanceF.h>

namespace IceInternal
{

class RetryTask : public IceUtil::TimerTask
{
public:
    
    RetryTask(const RetryQueuePtr&, const OutgoingAsyncPtr&);
    
    virtual void runTimerTask();
    void destroy();
    
    bool operator<(const RetryTask&) const;
    
private:
    
    const RetryQueuePtr _queue;
    const OutgoingAsyncPtr _outAsync;
};
typedef IceUtil::Handle<RetryTask> RetryTaskPtr;

class RetryQueue : public IceUtil::Shared, public IceUtil::Mutex
{
public:

    RetryQueue(const InstancePtr&);
    
    void add(const OutgoingAsyncPtr&, int);
    void destroy();

private:

    bool remove(const RetryTaskPtr&);
    friend class RetryTask;

    const InstancePtr _instance;
    std::set<RetryTaskPtr> _requests;
};

}

#endif

