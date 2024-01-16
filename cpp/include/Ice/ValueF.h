//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_VALUE_F_H
#define ICE_VALUE_F_H

#include <Ice/Config.h>
#include <Ice/SharedPtr.h>

namespace Ice
{
    class Value;

#ifdef ICE_CPP11_MAPPING

/// \cond INTERNAL
using ValuePtr = ::std::shared_ptr<Value>;
/// \endcond

#else

using ValuePtr = SharedPtr<Value>;

#endif

}

#endif
