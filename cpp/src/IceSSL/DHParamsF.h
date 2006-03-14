// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
