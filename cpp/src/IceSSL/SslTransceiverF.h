// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
