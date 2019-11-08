//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICESSL_API_EXPORTS
#   define ICESSL_API_EXPORTS
#endif
#include <IceSSL/ConnectionInfo.h>
#include <Ice/InputStream.h>
#include <Ice/OutputStream.h>

IceSSL::ConnectionInfo::~ConnectionInfo()
{
}

#ifndef ICE_CPP11_MAPPING

/// \cond INTERNAL
ICESSL_API ::Ice::LocalObject* IceSSL::upCast(ConnectionInfo* p) { return p; }
/// \endcond

#endif
