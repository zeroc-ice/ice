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

namespace IceSecurity
{

namespace Ssl
{

namespace OpenSSL
{

class RSAPrivateKey;
typedef IceInternal::Handle<RSAPrivateKey> RSAPrivateKeyPtr;

}

}

}

namespace IceInternal
{

void incRef(::IceSecurity::Ssl::OpenSSL::RSAPrivateKey*);
void decRef(::IceSecurity::Ssl::OpenSSL::RSAPrivateKey*);

}

#endif
