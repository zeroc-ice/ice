// **********************************************************************
//
// Copyright (c) 2002
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SSL_TRANSEIVER_OPENSSL_F_H
#define ICE_SSL_TRANSEIVER_OPENSSL_F_H

#include <Ice/Handle.h>

namespace IceSSL
{

class SslTransceiver;
typedef IceInternal::Handle<SslTransceiver> SslTransceiverPtr;

}

namespace IceInternal
{

void incRef(::IceSSL::SslTransceiver*);
void decRef(::IceSSL::SslTransceiver*);

}

#endif
