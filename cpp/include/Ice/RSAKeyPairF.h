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

namespace IceSSL
{

namespace OpenSSL
{

class RSAKeyPair;
typedef IceInternal::Handle<RSAKeyPair> RSAKeyPairPtr;

}

}

namespace IceInternal
{

void incRef(::IceSSL::OpenSSL::RSAKeyPair*);
void decRef(::IceSSL::OpenSSL::RSAKeyPair*);

}

#endif
