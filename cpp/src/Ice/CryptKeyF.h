// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_CRYPT_KEY_F_H
#define ICE_CRYPT_KEY_F_H

#include <Ice/Handle.h>

namespace IceSecurity
{

namespace SecureUdp
{

class CryptKey;
typedef IceInternal::Handle<CryptKey> CryptKeyPtr;

}

}

namespace IceInternal
{

void incRef(::IceSecurity::SecureUdp::CryptKey*);
void decRef(::IceSecurity::SecureUdp::CryptKey*);

}

#endif

