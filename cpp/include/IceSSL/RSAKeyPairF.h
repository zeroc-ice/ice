// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_RSA_KEY_PAIR_F_H
#define ICE_SSL_RSA_KEY_PAIR_F_H

#include <Ice/Handle.h>
#include <IceSSL/Config.h>

namespace IceSSL
{

class RSAKeyPair;
typedef IceInternal::Handle<RSAKeyPair> RSAKeyPairPtr;

}

namespace IceInternal
{

void ICE_SSL_API incRef(::IceSSL::RSAKeyPair*);
void ICE_SSL_API decRef(::IceSSL::RSAKeyPair*);

}

#endif
