// **********************************************************************
//
// Copyright (c) 2001
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

#ifndef SUBSCRIBER_FACTORY_H
#define SUBSCRIBER_FACTORY_H

#include <IceStorm/IceStormInternal.h> // For QoS, TopicLink

namespace IceStorm
{

//
// Forward declarations.
//
class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

class Subscriber;
typedef IceUtil::Handle<Subscriber> SubscriberPtr;

class Flusher;
typedef IceUtil::Handle<Flusher> FlusherPtr;

//
// Factory object that knows how to create various type of Subscriber
// objects.
//
class SubscriberFactory  : public IceUtil::Shared
{
public:

    SubscriberFactory(const TraceLevelsPtr&, const FlusherPtr&);

    //
    // Create a link subscriber (that is a subscriber that points to
    // another topic instance).
    //
    SubscriberPtr createLinkSubscriber(const TopicLinkPrx&, Ice::Int);

    //
    // Create a Subscriber with the given QoS.
    //
    SubscriberPtr createSubscriber(const QoS&, const Ice::ObjectPrx&);

private:

    TraceLevelsPtr _traceLevels;
    FlusherPtr _flusher;
};

typedef IceUtil::Handle<SubscriberFactory> SubscriberFactoryPtr;

} // End namespace IceStorm

#endif
