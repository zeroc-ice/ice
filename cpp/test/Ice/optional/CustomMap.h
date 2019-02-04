//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef CUSTOM_MAP_H
#define CUSTOM_MAP_H

#include <IceUtil/Config.h>

#ifdef ICE_CPP11_COMPILER
#   include <unordered_map>
#else
#   include <map>
#endif

namespace Test
{

#ifdef ICE_CPP11_COMPILER
template<typename K, typename V>
class CustomMap : public std::unordered_map<K, V>
{
};
#else
template<typename K, typename V>
class CustomMap : public std::map<K, V>
{
};
#endif

}

#endif
