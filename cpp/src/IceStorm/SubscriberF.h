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

void incRef(Subscriber*);
void decRef(Subscriber*);

} // End namespace IceStorm

#endif
