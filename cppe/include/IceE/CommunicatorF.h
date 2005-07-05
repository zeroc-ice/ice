// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_COMMUNICATOR_F_H
#define ICEE_COMMUNICATOR_F_H

#include <IceE/Handle.h>

namespace IceE
{

class Communicator;

}

namespace IceEInternal
{

ICEE_API void incRef(::IceE::Communicator*);
ICEE_API void decRef(::IceE::Communicator*);

}

namespace IceE
{

typedef ::IceEInternal::Handle< ::IceE::Communicator> CommunicatorPtr;

}

#endif
