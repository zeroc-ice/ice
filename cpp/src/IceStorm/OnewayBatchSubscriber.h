// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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

    virtual void unsubscribe();
    virtual void replace();
    virtual bool inactive() const;

    virtual void flush();

    virtual bool operator==(const Flushable&) const;

private:

    Ice::CommunicatorPtr _communicator;
    FlusherPtr _flusher;
};

typedef IceUtil::Handle<OnewayBatchSubscriber> OnewayBatchSubscriberPtr;

} // End namespace IceStorm

#endif
