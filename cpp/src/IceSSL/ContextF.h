// **********************************************************************
//
// Copyright (c) 2002
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

#ifndef ICE_SSL_CONTEXT_OPENSSL_F_H
#define ICE_SSL_CONTEXT_OPENSSL_F_H

#include <Ice/Handle.h>

namespace IceSSL
{

class Context;
typedef IceInternal::Handle<Context> ContextPtr;

}

namespace IceInternal
{

void incRef(::IceSSL::Context*);
void decRef(::IceSSL::Context*);

}

#endif
