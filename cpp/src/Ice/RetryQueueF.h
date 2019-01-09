// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef ICE_RETRY_QUEUE_F_H
#define ICE_RETRY_QUEUE_F_H

#include <Ice/Handle.h>

namespace IceInternal
{

class RetryQueue;
IceUtil::Shared* upCast(RetryQueue*);
typedef Handle<RetryQueue> RetryQueuePtr;

}

#endif
