// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef ICESSL_UWP_ENGINE_F_H
#define ICESSL_UWP_ENGINE_F_H

#include <IceUtil/Shared.h>
#include <Ice/Handle.h>
#include <IceSSL/UWP.h>

namespace IceSSL
{

namespace UWP
{

class SSLEngine;
ICESSL_API IceUtil::Shared* upCast(SSLEngine*);
typedef IceInternal::Handle<SSLEngine> SSLEnginePtr;

}

}

#endif
