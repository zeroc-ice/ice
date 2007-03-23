// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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

}

namespace IceInternal
{

IceUtil::Shared* upCast(IceSSL::TrustManager*);

}

namespace IceSSL
{

typedef IceInternal::Handle<TrustManager> TrustManagerPtr;

}

#endif
