// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
