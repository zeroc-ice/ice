//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_VALUE_F_H
#define ICE_VALUE_F_H

#include <Ice/Config.h>
#include <Ice/Handle.h>

namespace Ice
{
    class Value;

#ifdef ICE_CPP11_MAPPING

/// \cond INTERNAL
using ValuePtr = ::std::shared_ptr<Value>;
/// \endcond

#else

ICE_API Value* upCast(Value*);
typedef IceInternal::Handle<Value> ValuePtr;
/// \cond INTERNAL
ICE_API void _icePatchValuePtr(ValuePtr&, const ValuePtr&);
/// \endcond
#endif
}

#endif
