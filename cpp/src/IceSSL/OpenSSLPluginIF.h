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
