//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICESSL_API_EXPORTS
#   define ICESSL_API_EXPORTS
#endif

#include <IceSSL/EndpointInfo.h>
#include <Ice/InputStream.h>
#include <Ice/OutputStream.h>

#ifdef ICE_CPP11_MAPPING // C++11 mapping
IceSSL::EndpointInfo::~EndpointInfo()
{
}
#else
IceSSL::EndpointInfo::~EndpointInfo()
{
}

/// \cond INTERNAL
ICESSL_API ::Ice::LocalObject* IceSSL::upCast(EndpointInfo* p) { return p; }
/// \endcond

#endif
