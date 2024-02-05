//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef __Ice_ImplicitContextF_h__
#define __Ice_ImplicitContextF_h__

#include <Ice/ProxyF.h>
#include <Ice/ObjectF.h>
#include <Ice/ValueF.h>
#include <Ice/Exception.h>
#include <Ice/LocalObject.h>
#include <Ice/StreamHelpers.h>
#include <Ice/Comparable.h>
#include <optional>
#include <IceUtil/UndefSysMacros.h>

namespace Ice
{

class ImplicitContext;

}

/// \cond INTERNAL
namespace Ice
{

using ImplicitContextPtr = ::std::shared_ptr<ImplicitContext>;

}
/// \endcond

#endif
