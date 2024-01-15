//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_DEFAULT_VALUE_FACTORY_H
#define ICE_DEFAULT_VALUE_FACTORY_H

#include <Ice/Config.h>
#include <Ice/ValueFactory.h>

namespace IceInternal
{

template<class V>
::std::shared_ptr<::Ice::Value>
#ifdef NDEBUG
defaultValueFactory(const std::string&)
#else
defaultValueFactory(const std::string& typeId)
#endif
{
    assert(typeId == V::ice_staticId());
    return std::make_shared<V>();
}

}
#endif
