// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SSL_CONNECTION_OPENSSL_F_H
#define ICE_SSL_CONNECTION_OPENSSL_F_H

#include <Ice/Handle.h>

namespace IceSecurity
{

namespace Ssl
{

namespace OpenSSL
{

class Connection;
typedef IceInternal::Handle<Connection> ConnectionPtr;

}

}

}

namespace IceInternal
{

void incRef(::IceSecurity::Ssl::OpenSSL::Connection*);
void decRef(::IceSecurity::Ssl::OpenSSL::Connection*);

}

#endif
