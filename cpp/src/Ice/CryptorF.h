// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_CRYPTOR_F_H
#define ICE_CRYPTOR_F_H

#include <Ice/Handle.h>

namespace SecureUdp
{

class Cryptor;
typedef IceInternal::Handle<Cryptor> CryptorPtr;

}

namespace IceInternal
{

void incRef(::SecureUdp::Cryptor*);
void decRef(::SecureUdp::Cryptor*);

}

#endif

