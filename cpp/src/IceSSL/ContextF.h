// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
