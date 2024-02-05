//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_USER_EXCEPTION_FACTORY_H
#define ICE_USER_EXCEPTION_FACTORY_H

#include <IceUtil/Shared.h>
#include <IceUtil/Handle.h>
#include <Ice/Config.h>

namespace Ice
{

/** Creates and throws a user exception. */
using UserExceptionFactory = std::function<void(const std::string&)>;

}

namespace IceInternal
{

template<class E>
void
#ifdef NDEBUG
defaultUserExceptionFactory(const std::string&)
#else
defaultUserExceptionFactory(const std::string& typeId)
#endif
{
    assert(typeId == E::ice_staticId());
    throw E();
}

}

#endif
