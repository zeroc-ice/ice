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

namespace IceSSL
{

class SystemInternal;
typedef IceInternal::Handle<SystemInternal> SystemInternalPtr;

}

namespace IceInternal
{

void incRef(IceSSL::SystemInternal*);
void decRef(IceSSL::SystemInternal*);

}

#endif
