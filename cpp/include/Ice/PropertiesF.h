//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef __Ice_PropertiesF_h__
#define __Ice_PropertiesF_h__

#include <IceUtil/PushDisableWarnings.h>
#include <Ice/ProxyF.h>
#include <Ice/ObjectF.h>
#include <Ice/ValueF.h>
#include <Ice/Exception.h>
#include <Ice/LocalObject.h>
#include <Ice/StreamHelpers.h>
#include <Ice/Comparable.h>
#include <Ice/Proxy.h>
#include <IceUtil/ScopedArray.h>
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

#ifdef ICE_CPP11_MAPPING // C++11 mapping

namespace Ice
{

class Properties;
class PropertiesAdmin;
class PropertiesAdminPrx;

}

/// \cond STREAM
namespace Ice
{

}
/// \endcond

/// \cond INTERNAL
namespace Ice
{

using PropertiesPtr = ::std::shared_ptr<Properties>;

}
/// \endcond

#else // C++98 mapping

namespace Ice
{

class Properties;
/// \cond INTERNAL
ICE_API LocalObject* upCast(Properties*);
/// \endcond
typedef ::IceInternal::Handle< Properties> PropertiesPtr;

}

#endif

#include <IceUtil/PopDisableWarnings.h>
#endif
