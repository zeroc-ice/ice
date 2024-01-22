//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICESSL_API_EXPORTS
#   define ICESSL_API_EXPORTS
#endif
#include <IceSSL/EndpointInfo.h>
#include <IceUtil/PushDisableWarnings.h>
#include <Ice/InputStream.h>
#include <Ice/OutputStream.h>
#include <IceUtil/PopDisableWarnings.h>

#if defined(_MSC_VER)
#   pragma warning(disable:4458) // declaration of ... hides class member
#elif defined(__clang__)
#   pragma clang diagnostic ignored "-Wshadow"
#elif defined(__GNUC__)
#   pragma GCC diagnostic ignored "-Wshadow"
#endif

#ifdef ICE_CPP11_MAPPING // C++11 mapping

IceSSL::EndpointInfo::~EndpointInfo()
{
}

#else // C++98 mapping

IceSSL::EndpointInfo::~EndpointInfo()
{
}

/// \cond INTERNAL
ICESSL_API ::Ice::LocalObject* IceSSL::upCast(EndpointInfo* p) { return p; }
/// \endcond

#endif
