// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_COMMUNICATOR_F_H
#define ICEE_COMMUNICATOR_F_H

#include <IceE/Handle.h>
#include <IceE/Shared.h>

namespace Ice
{

class Communicator;
inline bool operator==(const Communicator& l, const Communicator& r)
{
    return &l == &r;
}
inline bool operator<(const Communicator& l, const Communicator& r)
{
    return &l < &r;
}

}

namespace IceInternal
{

ICE_API IceUtil::Shared* upCast(::Ice::Communicator*);

}

namespace Ice
{

typedef ::IceInternal::Handle< ::Ice::Communicator> CommunicatorPtr;

}

#endif
