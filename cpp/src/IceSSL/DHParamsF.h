// **********************************************************************
//
// Copyright (c) 2002
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_DH_PARAMS_F_H
#define ICE_DH_PARAMS_F_H

#include <Ice/Handle.h>

namespace IceSSL
{

class DHParams;
typedef IceInternal::Handle<DHParams> DHParamsPtr;

}

namespace IceInternal
{

void incRef(::IceSSL::DHParams*);
void decRef(::IceSSL::DHParams*);

}

#endif
