// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef SUBSCRIBER_F_H
#define SUBSCRIBER_F_H

#include <IceUtil/Handle.h>

namespace IceStorm
{

class Subscriber;

typedef IceUtil::Handle<Subscriber> SubscriberPtr;

} // End namespace IceStorm

//TODO:
#ifdef never
//
// This is necessary so that SubscriberPtr can be forward declared
//
namespace IceInternal
{

void incRef(IceStorm::Subscriber*);
void decRef(IceStorm::Subscriber*);

} // End namespace IceInternal
#endif

#endif
