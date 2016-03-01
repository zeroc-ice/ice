// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_TRUST_MANAGER_F_H
#define ICE_SSL_TRUST_MANAGER_F_H

#include <IceUtil/Shared.h>

#include <Ice/Handle.h>

namespace IceSSL
{

class TrustManager;
IceUtil::Shared* upCast(IceSSL::TrustManager*);
typedef IceInternal::Handle<TrustManager> TrustManagerPtr;

}

#endif
