//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICESSL_CONNECTION_INFOF_H
#define ICESSL_CONNECTION_INFOF_H

#include <Ice/ProxyF.h>
#include <Ice/ObjectF.h>
#include <Ice/ValueF.h>
#include <Ice/Exception.h>
#include <Ice/LocalObject.h>
#include <Ice/StreamHelpers.h>
#include <Ice/Comparable.h>
#include <IceUtil/ScopedArray.h>
#include <Ice/Optional.h>

#ifndef ICESSL_API
#   if defined(ICE_STATIC_LIBS)
#       define ICESSL_API /**/
#   elif defined(ICESSL_API_EXPORTS)
#       define ICESSL_API ICE_DECLSPEC_EXPORT
#   else
#       define ICESSL_API ICE_DECLSPEC_IMPORT
#   endif
#endif

#ifdef ICE_CPP11_MAPPING // C++11 mapping

namespace IceSSL
{

class ConnectionInfo;
using ConnectionInfoPtr = ::std::shared_ptr<ConnectionInfo>;

}

#else // C++98 mapping

namespace IceSSL
{

class ConnectionInfo;
/// \cond INTERNAL
ICESSL_API ::Ice::LocalObject* upCast(ConnectionInfo*);
/// \endcond
typedef ::IceInternal::Handle< ConnectionInfo> ConnectionInfoPtr;

}

#endif

#endif
