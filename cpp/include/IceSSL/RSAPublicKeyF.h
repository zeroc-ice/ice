// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_RSA_PUBLIC_KEY_F_H
#define ICE_SSL_RSA_PUBLIC_KEY_F_H

#include <Ice/Handle.h>

namespace IceSSL
{

class RSAPublicKey;
typedef IceInternal::Handle<RSAPublicKey> RSAPublicKeyPtr;

}

namespace IceInternal
{

void incRef(::IceSSL::RSAPublicKey*);
void decRef(::IceSSL::RSAPublicKey*);

}

#endif
