// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
