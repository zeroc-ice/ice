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

namespace SecureUdp
{

class CryptKey;
typedef IceInternal::Handle<CryptKey> CryptKeyPtr;

}

namespace IceInternal
{

void incRef(::SecureUdp::CryptKey*);
void decRef(::SecureUdp::CryptKey*);

}

#endif

