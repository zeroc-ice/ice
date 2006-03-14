// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_RSA_PRIVATE_KEY_F_H
#define ICE_SSL_RSA_PRIVATE_KEY_F_H

#include <Ice/Handle.h>

namespace IceSSL
{

class RSAPrivateKey;
typedef IceInternal::Handle<RSAPrivateKey> RSAPrivateKeyPtr;

}

namespace IceInternal
{

void incRef(::IceSSL::RSAPrivateKey*);
void decRef(::IceSSL::RSAPrivateKey*);

}

#endif
