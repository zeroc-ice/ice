// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ONEWAY_SUBSCRIBER_H
#define ONEWAY_SUBSCRIBER_H

#include <IceStorm/Subscriber.h>

namespace IceStorm
{

class OnewaySubscriber : public Subscriber
{
public:

    OnewaySubscriber(const TraceLevelsPtr&, const Ice::ObjectPrx&);
    ~OnewaySubscriber();

    virtual bool persistent() const;
    virtual void unsubscribe();
    virtual void replace();
    virtual void publish(const Event&);

protected:

    // Immutable
    Ice::ObjectPrx _obj;
};

} // End namespace IceStorm

#endif
