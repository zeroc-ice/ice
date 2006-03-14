// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
