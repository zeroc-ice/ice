// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SSL_PLUGIN_BASE_I_F_H
#define ICE_SSL_PLUGIN_BASE_I_F_H

#include <Ice/Handle.h>

namespace IceSSL
{

class PluginBaseI;
typedef IceInternal::Handle<PluginBaseI> PluginBaseIPtr;

}

namespace IceInternal
{

void incRef(IceSSL::PluginBaseI*);
void decRef(IceSSL::PluginBaseI*);

}

#endif
