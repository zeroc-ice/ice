// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_RSA_KEY_PAIR_F_H
#define ICE_RSA_KEY_PAIR_F_H

#include <Ice/Handle.h>

namespace IceSecurity
{

namespace Ssl
{

namespace OpenSSL
{

class RSAKeyPair;
typedef IceInternal::Handle<RSAKeyPair> RSAKeyPairPtr;

}

}

}

namespace IceInternal
{

void incRef(::IceSecurity::Ssl::OpenSSL::RSAKeyPair*);
void decRef(::IceSecurity::Ssl::OpenSSL::RSAKeyPair*);

}

#endif
