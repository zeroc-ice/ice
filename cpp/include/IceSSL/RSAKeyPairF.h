// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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

namespace OpenSSL
{

class RSAKeyPair;
typedef IceInternal::Handle<RSAKeyPair> RSAKeyPairPtr;

}

}

namespace IceInternal
{

void ICE_SSL_API incRef(::IceSSL::OpenSSL::RSAKeyPair*);
void ICE_SSL_API decRef(::IceSSL::OpenSSL::RSAKeyPair*);

}

#endif
