// **********************************************************************
//
// Copyright (c) 2002
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
