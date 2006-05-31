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

// XXX: This does not need to be exported.
#ifndef ICE_SSL_API
#   ifdef ICE_SSL_API_EXPORTS
#       define ICE_SSL_API ICE_DECLSPEC_EXPORT
#    else
#       define ICE_SSL_API ICE_DECLSPEC_IMPORT
#    endif
#endif

namespace IceSSL
{

class TrustManager;

}

namespace IceInternal
{

ICE_SSL_API void incRef(IceSSL::TrustManager*);
ICE_SSL_API void decRef(IceSSL::TrustManager*);

}

namespace IceSSL
{

typedef IceInternal::Handle<TrustManager> TrustManagerPtr;

}

#endif
