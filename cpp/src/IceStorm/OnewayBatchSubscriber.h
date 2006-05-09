// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ONEWAY_BATCH_SUBSCRIBER_H
#define ONEWAY_BATCH_SUBSCRIBER_H

#include <IceStorm/OnewaySubscriber.h>
#include <IceStorm/Flushable.h>
#include <IceStorm/QueuedProxy.h>

namespace IceStorm
{

//
// Forward declarations.
//
class Flusher;
typedef IceUtil::Handle<Flusher> FlusherPtr;

class OnewayBatchSubscriber : public OnewaySubscriber, public Flushable
{
public:

    OnewayBatchSubscriber(const SubscriberFactoryPtr&, const Ice::CommunicatorPtr&,
	                  const TraceLevelsPtr&, const FlusherPtr&, const QueuedProxyPtr&);
    ~OnewayBatchSubscriber();

    virtual void activate();
    virtual void unsubscribe();
    virtual void replace();
    virtual bool inactive() const;

    virtual void flush();

    virtual bool operator==(const Flushable&) const;

private:

    FlusherPtr _flusher;
};

typedef IceUtil::Handle<OnewayBatchSubscriber> OnewayBatchSubscriberPtr;

} // End namespace IceStorm

#endif
