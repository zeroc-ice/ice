// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_RSA_PUBLIC_KEY_F_H
#define ICE_RSA_PUBLIC_KEY_F_H

#include <Ice/Handle.h>

namespace IceSecurity
{

namespace Ssl
{

namespace OpenSSL
{

class RSAPublicKey;
typedef IceInternal::Handle<RSAPublicKey> RSAPublicKeyPtr;

}

}

}

namespace IceInternal
{

void incRef(::IceSecurity::Ssl::OpenSSL::RSAPublicKey*);
void decRef(::IceSecurity::Ssl::OpenSSL::RSAPublicKey*);

}

#endif
