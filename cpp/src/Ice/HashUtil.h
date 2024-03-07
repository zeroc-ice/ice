//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_HASH_UTIL_H
#define ICE_HASH_UTIL_H

namespace IceInternal
{

    inline void hashAdd(std::int32_t& hashCode, std::int32_t value)
    {
        hashCode = ((hashCode << 5) + hashCode) ^ static_cast<std::int32_t>(2654435761u) * value;
    }

    inline void hashAdd(std::int32_t& hashCode, bool value)
    {
        hashCode = ((hashCode << 5) + hashCode) ^ (value ? 1 : 0);
    }

    inline void hashAdd(std::int32_t& hashCode, std::byte value)
    {
        // TODO: better hash function for std::byte
        hashAdd(hashCode, static_cast<std::int32_t>(value));
    }

    inline void hashAdd(std::int32_t& hashCode, const std::string& value)
    {
        for (std::string::const_iterator p = value.begin(); p != value.end(); ++p)
        {
            hashCode = ((hashCode << 5) + hashCode) ^ *p;
        }
    }

    template<typename T> void hashAdd(std::int32_t& hashCode, const std::vector<T>& seq)
    {
        for (typename std::vector<T>::const_iterator p = seq.begin(); p != seq.end(); ++p)
        {
            hashAdd(hashCode, *p);
        }
    }

    template<typename K, typename V> void hashAdd(std::int32_t& hashCode, const std::map<K, V>& map)
    {
        for (typename std::map<K, V>::const_iterator p = map.begin(); p != map.end(); ++p)
        {
            hashAdd(hashCode, p->first);
            hashAdd(hashCode, p->second);
        }
    }

}

#endif
