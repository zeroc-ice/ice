// Copyright (c) ZeroC, Inc.

#ifndef ICE_HASH_UTIL_H
#define ICE_HASH_UTIL_H

#include "EndpointI.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <vector>

// All these functions are for hashing proxies and endpoints. They are not general purpose.

namespace IceInternal
{
    inline void hashAdd(std::size_t& hashCode, std::size_t value)
    {
        hashCode = ((hashCode << 5) + hashCode) ^ 2654435761u * value;
    }

    inline void hashAdd(std::size_t& hashCode, std::int32_t value)
    {
        hashAdd(hashCode, static_cast<std::size_t>(value));
    }

    inline void hashAdd(std::size_t& hashCode, bool value)
    {
        hashCode = ((hashCode << 5) + hashCode) ^ (value ? 1 : 0);
    }

    template<class T> inline void hashAdd(std::size_t& hashCode, const T& value)
    {
        hashAdd(hashCode, std::hash<T>{}(value));
    }

    inline void hashAdd(std::size_t& hashCode, const std::vector<EndpointIPtr>& seq)
    {
        for (const auto& p : seq)
        {
            hashAdd(hashCode, *p);
        }
    }

    inline void hashAdd(std::size_t& hashCode, const std::vector<std::byte>& seq)
    {
        // TODO: better hash function for vector<byte> (used for opaque endpoints).
        for (const auto& p : seq)
        {
            hashAdd(hashCode, static_cast<std::size_t>(p));
        }
    }

    // For Ice::Context
    inline void hashAdd(std::size_t& hashCode, const std::map<std::string, std::string, std::less<>>& map)
    {
        for (const auto& p : map)
        {
            hashAdd(hashCode, p.first);
            hashAdd(hashCode, p.second);
        }
    }
}

#endif
