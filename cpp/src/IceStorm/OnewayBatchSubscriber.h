// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ONEWAY_BATCH_SUBSCRIBER_H
#define ONEWAY_BATCH_SUBSCRIBER_H

#include <IceStorm/OnewaySubscriber.h>
#include <IceStorm/Flushable.h>

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

    OnewayBatchSubscriber(const TraceLevelsPtr&, const FlusherPtr&, const Ice::ObjectPrx&);
    ~OnewayBatchSubscriber();

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
