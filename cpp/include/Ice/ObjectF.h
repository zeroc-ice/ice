//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_OBJECT_F_H
#define ICE_OBJECT_F_H

#include <Ice/Config.h>
#include <memory>

namespace Ice
{

class Object;
/// \cond INTERNAL
using ObjectPtr = ::std::shared_ptr<Object>;
/// \endcond

}

#endif
