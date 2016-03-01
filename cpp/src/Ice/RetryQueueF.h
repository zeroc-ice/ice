// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
