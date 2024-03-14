//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_EXCEPTION_H
#define ICE_EXCEPTION_H

#include "IceUtil/Exception.h"
#include "Config.h"

namespace Ice
{
    class Value;
    using Exception = IceUtil::Exception;
}

namespace IceInternal
{
    namespace Ex
    {
        ICE_API void throwUOE(const std::string&, const std::shared_ptr<Ice::Value>&);
        ICE_API void throwMemoryLimitException(const char*, int, size_t, size_t);
        ICE_API void throwMarshalException(const char*, int, std::string);
    }
}

#endif
