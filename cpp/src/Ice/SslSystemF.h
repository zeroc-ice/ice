// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SSL_SYSTEM_F_H
#define ICE_SSL_SYSTEM_F_H

#include <Ice/Handle.h>

namespace IceSecurity
{

namespace Ssl
{

class System;
typedef IceInternal::Handle<System> SystemPtr;

}

}

namespace IceInternal
{

void incRef(IceSecurity::Ssl::System*);
void decRef(IceSecurity::Ssl::System*);

}

#endif
