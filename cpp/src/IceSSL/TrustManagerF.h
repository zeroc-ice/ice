// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_TRUST_MANAGER_F_H
#define ICE_SSL_TRUST_MANAGER_F_H

#include <Ice/Handle.h>

namespace IceSSL
{

class TrustManager;

}

namespace IceInternal
{

void incRef(IceSSL::TrustManager*);
void decRef(IceSSL::TrustManager*);

}

namespace IceSSL
{

typedef IceInternal::Handle<TrustManager> TrustManagerPtr;

}

#endif
