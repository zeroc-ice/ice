// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
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

namespace OpenSSL
{

class Context;
typedef IceInternal::Handle<Context> ContextPtr;

}

}

namespace IceInternal
{

void incRef(::IceSSL::OpenSSL::Context*);
void decRef(::IceSSL::OpenSSL::Context*);

}

#endif
