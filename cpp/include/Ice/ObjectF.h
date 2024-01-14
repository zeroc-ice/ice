//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_OBJECT_F_H
#define ICE_OBJECT_F_H

#include <Ice/Config.h>
#include <Ice/SharedPtr.h>

namespace Ice
{

class Object;

#ifdef ICE_CPP11_MAPPING
/// \cond INTERNAL
using ObjectPtr = ::std::shared_ptr<Object>;
/// \endcond
#else
using ObjectPtr = SharedPtr<Object>;
#endif

}

#endif
