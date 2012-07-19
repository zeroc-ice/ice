// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

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
