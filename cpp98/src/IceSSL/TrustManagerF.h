//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICESSL_TRUST_MANAGER_F_H
#define ICESSL_TRUST_MANAGER_F_H

#include <IceUtil/Shared.h>

#include <Ice/Handle.h>

namespace IceSSL
{

class TrustManager;
IceUtil::Shared* upCast(IceSSL::TrustManager*);
typedef IceInternal::Handle<TrustManager> TrustManagerPtr;

}

#endif
