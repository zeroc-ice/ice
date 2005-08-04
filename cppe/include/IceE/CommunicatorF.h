// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_COMMUNICATOR_F_H
#define ICEE_COMMUNICATOR_F_H

#include <IceE/Handle.h>

namespace Ice
{

class Communicator;

}

namespace IceInternal
{

ICEE_API void incRef(::Ice::Communicator*);
ICEE_API void decRef(::Ice::Communicator*);

}

namespace Ice
{

typedef ::IceInternal::Handle< ::Ice::Communicator> CommunicatorPtr;

}

#endif
