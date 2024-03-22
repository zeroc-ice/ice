//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_EXCEPTION_H
#define ICE_EXCEPTION_H

#include "Config.h"
#include "IceUtil/Exception.h"
#include "ValueF.h"

#include <memory>

namespace Ice
{
    using Exception = IceUtil::Exception;
}

namespace IceInternal::Ex
{
    ICE_API void throwUOE(const std::string&, const Ice::ValuePtr&);
    ICE_API void throwMemoryLimitException(const char*, int, size_t, size_t);
    ICE_API void throwMarshalException(const char*, int, std::string);
}

#endif
