// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_RSA_PRIVATE_KEY_F_H
#define ICE_RSA_PRIVATE_KEY_F_H

#include <Ice/Handle.h>

namespace IceSSL
{

namespace OpenSSL
{

class RSAPrivateKey;
typedef IceInternal::Handle<RSAPrivateKey> RSAPrivateKeyPtr;

}

}

namespace IceInternal
{

void incRef(::IceSSL::OpenSSL::RSAPrivateKey*);
void decRef(::IceSSL::OpenSSL::RSAPrivateKey*);

}

#endif
