//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_USER_EXCEPTION_FACTORY_H
#define ICE_USER_EXCEPTION_FACTORY_H

#include <Ice/Config.h>

namespace Ice
{
    /** Creates and throws a user exception. */
    using UserExceptionFactory = std::function<void(std::string_view)>;
}

namespace IceInternal
{
    template<class E>
    void
#ifdef NDEBUG
    defaultUserExceptionFactory(std::string_view)
#else
    defaultUserExceptionFactory(std::string_view typeId)
#endif
    {
        assert(typeId == E::ice_staticId());
        throw E();
    }
}

#endif
