// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef ICE_SSL_RSA_KEY_PAIR_F_H
#define ICE_SSL_RSA_KEY_PAIR_F_H

#include <Ice/Handle.h>

#ifdef _WIN32
#   ifdef ICE_SSL_API_EXPORTS
#       define ICE_SSL_API __declspec(dllexport)
#   else
#       define ICE_SSL_API __declspec(dllimport)
#   endif
#else
#   define ICE_SSL_API /**/
#endif

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
