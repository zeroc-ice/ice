// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
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

namespace OpenSSL
{

class DHParams;
typedef IceInternal::Handle<DHParams> DHParamsPtr;

}

}

namespace IceInternal
{

void incRef(::IceSSL::OpenSSL::DHParams*);
void decRef(::IceSSL::OpenSSL::DHParams*);

}

#endif
