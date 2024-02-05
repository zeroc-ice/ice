//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef __Ice_EndpointSelectionType_h__
#define __Ice_EndpointSelectionType_h__

#include <Ice/ProxyF.h>
#include <Ice/ObjectF.h>
#include <Ice/ValueF.h>
#include <Ice/Exception.h>
#include <Ice/LocalObject.h>
#include <Ice/StreamHelpers.h>
#include <Ice/Comparable.h>
#include <Ice/Optional.h>
#include <IceUtil/UndefSysMacros.h>

#ifndef ICE_API
#   if defined(ICE_STATIC_LIBS)
#       define ICE_API /**/
#   elif defined(ICE_API_EXPORTS)
#       define ICE_API ICE_DECLSPEC_EXPORT
#   else
#       define ICE_API ICE_DECLSPEC_IMPORT
#   endif
#endif

namespace Ice
{

/**
 * Determines the order in which the Ice run time uses the endpoints in a proxy when establishing a connection.
 */
enum class EndpointSelectionType : unsigned char
{
    /**
     * <code>Random</code> causes the endpoints to be arranged in a random order.
     */
    Random,
    /**
     * <code>Ordered</code> forces the Ice run time to use the endpoints in the order they appeared in the proxy.
     */
    Ordered
};

}

#endif
