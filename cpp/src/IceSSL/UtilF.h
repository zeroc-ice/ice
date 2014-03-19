// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_UTIL_F_H
#define ICE_SSL_UTIL_F_H

#include <IceUtil/Shared.h>

#include <Ice/Handle.h>

#include <openssl/ssl.h>

#ifndef OPENSSL_NO_DH
namespace IceSSL
{

class DHParams;
IceUtil::Shared* upCast(IceSSL::DHParams*);
typedef IceInternal::Handle<DHParams> DHParamsPtr;

}
#endif

#endif
