// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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
#include <Ice/RequestHandler.h> // For CancellationHandler

namespace IceInternal
{

class RetryTask : public IceUtil::TimerTask, public CancellationHandler
{
public:

    RetryTask(const InstancePtr&, const RetryQueuePtr&, const ProxyOutgoingAsyncBasePtr&);

    virtual void runTimerTask();

    virtual void requestCanceled(OutgoingBase*, const Ice::LocalException&);
    virtual void asyncRequestCanceled(const OutgoingAsyncBasePtr&, const Ice::LocalException&);

    void destroy();

    bool operator<(const RetryTask&) const;


private:

    const InstancePtr _instance;
    const RetryQueuePtr _queue;
    const ProxyOutgoingAsyncBasePtr _outAsync;
};
typedef IceUtil::Handle<RetryTask> RetryTaskPtr;

class RetryQueue : public IceUtil::Shared, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    RetryQueue(const InstancePtr&);

    void add(const ProxyOutgoingAsyncBasePtr&, int);
    void destroy();

private:

    void remove(const RetryTaskPtr&);
    bool cancel(const RetryTaskPtr&);
    friend class RetryTask;

    InstancePtr _instance;
    std::set<RetryTaskPtr> _requests;
};

}

#endif

