//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_OBJECT_F_H
#define ICE_OBJECT_F_H

#include <IceUtil/Shared.h>
#include <Ice/Handle.h>

namespace Ice
{

class Object;
/// \cond INTERNAL
using ObjectPtr = ::std::shared_ptr<Object>;
/// \endcond

}

#endif
