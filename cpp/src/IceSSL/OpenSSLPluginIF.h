// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SSL_OPENSSL_PLUGIN_I_F_H
#define ICE_SSL_OPENSSL_PLUGIN_I_F_H

#include <Ice/Handle.h>

namespace IceSSL
{

class OpenSSLPluginI;
typedef IceInternal::Handle<OpenSSLPluginI> OpenSSLPluginIPtr;

}

namespace IceInternal
{

void incRef(IceSSL::OpenSSLPluginI*);
void decRef(IceSSL::OpenSSLPluginI*);

}

#endif
