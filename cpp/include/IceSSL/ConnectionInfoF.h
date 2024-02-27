//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef __IceSSL_ConnectionInfoF_h__
#define __IceSSL_ConnectionInfoF_h__

#include <Ice/ProxyF.h>
#include <Ice/ObjectF.h>
#include <Ice/ValueF.h>
#include <Ice/Exception.h>
#include <Ice/StreamHelpers.h>
#include <Ice/Comparable.h>
#include <optional>
#include <IceUtil/UndefSysMacros.h>

#ifndef ICESSL_API
#    if defined(ICE_STATIC_LIBS)
#        define ICESSL_API /**/
#    elif defined(ICESSL_API_EXPORTS)
#        define ICESSL_API ICE_DECLSPEC_EXPORT
#    else
#        define ICESSL_API ICE_DECLSPEC_IMPORT
#    endif
#endif

namespace IceSSL
{

    class ConnectionInfo;

}

/// \cond INTERNAL
namespace IceSSL
{

    using ConnectionInfoPtr = ::std::shared_ptr<ConnectionInfo>;

}
/// \endcond

#endif
