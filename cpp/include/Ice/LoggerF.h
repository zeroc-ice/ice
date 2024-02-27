//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef __Ice_LoggerF_h__
#define __Ice_LoggerF_h__

#include <Ice/ProxyF.h>
#include <Ice/ObjectF.h>
#include <Ice/ValueF.h>
#include <Ice/Exception.h>
#include <Ice/StreamHelpers.h>
#include <Ice/Comparable.h>
#include <optional>
#include <IceUtil/UndefSysMacros.h>

#ifndef ICE_API
#    if defined(ICE_STATIC_LIBS)
#        define ICE_API /**/
#    elif defined(ICE_API_EXPORTS)
#        define ICE_API ICE_DECLSPEC_EXPORT
#    else
#        define ICE_API ICE_DECLSPEC_IMPORT
#    endif
#endif

namespace Ice
{

    class Logger;

}

/// \cond INTERNAL
namespace Ice
{

    using LoggerPtr = ::std::shared_ptr<Logger>;

}
/// \endcond

#endif
