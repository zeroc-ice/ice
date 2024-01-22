//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef __Ice_ImplicitContextF_h__
#define __Ice_ImplicitContextF_h__

#include <IceUtil/PushDisableWarnings.h>
#include <Ice/ProxyF.h>
#include <Ice/ObjectF.h>
#include <Ice/ValueF.h>
#include <Ice/Exception.h>
#include <Ice/LocalObject.h>
#include <Ice/StreamHelpers.h>
#include <Ice/Comparable.h>
#include <IceUtil/ScopedArray.h>
#include <Ice/Optional.h>
#include <IceUtil/UndefSysMacros.h>

#ifdef ICE_CPP11_MAPPING // C++11 mapping

namespace Ice
{

class ImplicitContext;

}

/// \cond STREAM
namespace Ice
{

}
/// \endcond

/// \cond INTERNAL
namespace Ice
{

using ImplicitContextPtr = ::std::shared_ptr<ImplicitContext>;

}
/// \endcond

#else // C++98 mapping

namespace Ice
{

class ImplicitContext;
/// \cond INTERNAL
ICE_API LocalObject* upCast(ImplicitContext*);
/// \endcond
typedef ::IceInternal::Handle< ImplicitContext> ImplicitContextPtr;

}

/// \cond STREAM
namespace Ice
{

}
/// \endcond

#endif

#include <IceUtil/PopDisableWarnings.h>
#endif
